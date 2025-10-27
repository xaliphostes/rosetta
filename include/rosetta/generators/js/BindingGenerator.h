// ============================================================================
// rosetta/generators/javascript/napi_binding_generator.h
//
// Générateur de bindings N-API à l'exécution (runtime)
// Utilise l'introspection Rosetta pour créer automatiquement les bindings
// ============================================================================
#pragma once

#include "../../core/registry.h"
#include "TypeConverterRegistry.h"
#include <memory>
#include <napi.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace rosetta::generators::js {

    /**
     * @brief Générateur de bindings N-API à l'exécution
     *
     * Crée automatiquement les bindings JavaScript pour les classes enregistrées
     * dans Rosetta, sans génération de code préalable.
     */
    class BindingGenerator {
        Napi::Env              env_;
        Napi::Object           exports_;
        TypeConverterRegistry &converter_registry_;

        // Store ctors
        std::unordered_map<std::string, Napi::FunctionReference> class_constructors_;

        // Store a type-erased factory function for each registered class
        std::unordered_map<std::type_index, std::function<Napi::Object(Napi::Env, const void *)>>
            class_factories_;

        std::unordered_map<std::type_index, std::string> type_to_classname_;

    public:
        /**
         * @brief Constructeur
         * @param env Environnement N-API
         * @param exports Objet exports du module
         */
        BindingGenerator(Napi::Env env, Napi::Object exports)
            : env_(env), exports_(exports), converter_registry_(TypeConverterRegistry::instance()) {
        }

        /**
         * @brief Lie une classe unique
         * @tparam Class Type de la classe
         * @param custom_name Nom personnalisé (optionnel)
         * @return Référence à this pour chaînage
         */
        template <typename Class>
        BindingGenerator &bind_class(const std::string &custom_name = "") {
            auto &registry = core::Registry::instance();

            if (!registry.has_class<Class>()) {
                throw std::runtime_error("Class not registered in Rosetta: " +
                                         std::string(typeid(Class).name()));
            }

            const auto &meta        = registry.get<Class>();
            const auto &inheritance = meta.inheritance();

            std::string class_name = custom_name.empty() ? meta.name() : custom_name;

            create_class<Class>(class_name, meta, inheritance);

            // Register the factory for this class type
            register_class_factory<Class>(class_name);

            return *this;
        }

        /**
         * @brief Lie plusieurs classes
         * @tparam Classes Types des classes
         * @return Référence à this pour chaînage
         */
        template <typename... Classes> BindingGenerator &bind_classes() {
            (bind_class<Classes>(), ...);
            return *this;
        }

        /**
         * @brief Lie une fonction globale
         * @tparam Ret Type de retour
         * @tparam Args Types des arguments
         * @param func Pointeur vers la fonction
         * @param name Nom de la fonction en JavaScript
         * @return Référence à this pour chaînage
         */
        template <typename Ret, typename... Args>
        BindingGenerator &bind_function(Ret (*func)(Args...), const std::string &name) {
            auto wrapper = [&, func](const Napi::CallbackInfo &info) -> Napi::Value {
                Napi::Env env = info.Env();

                // Vérifier le nombre d'arguments
                if (info.Length() != sizeof...(Args)) {
                    Napi::TypeError::New(env, "Wrong number of arguments")
                        .ThrowAsJavaScriptException();
                    return env.Undefined();
                }

                try {
                    // Convertir les arguments
                    auto args_tuple =
                        convert_args<Args...>(info, std::index_sequence_for<Args...>{});

                    // Appeler la fonction
                    if constexpr (std::is_void_v<Ret>) {
                        std::apply(func, args_tuple);
                        return env.Undefined();
                    } else {
                        Ret result = std::apply(func, args_tuple);
                        return converter_registry_.to_napi(env, result);
                    }
                } catch (const std::exception &e) {
                    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
                    return env.Undefined();
                }
            };

            exports_.Set(name, Napi::Function::New(env_, wrapper, name));

            return *this;
        }

        template <typename T>
        inline T unwrap_object_as(Napi::Env env, Napi::Value js) {
            if (!js.IsObject())
                throw rosetta::generators::js::NapiConversionError("Expected object");
            auto obj = js.As<Napi::Object>();
            // This works without knowing the ctor: ObjectWrap can unwrap by T
            auto *w = rosetta::generators::js::BindingGenerator::ClassWrapper<T>::Unwrap(obj);
            if (!w)
                throw rosetta::generators::js::NapiConversionError("Wrong wrapped type");
            return w->get_instance(); // returns by value (copy), OK for const& params
        }

        /**
         * @brief Lie une énumération
         * @tparam Enum Type de l'énumération
         * @param name Nom de l'énumération
         * @param values Liste des valeurs {nom, valeur}
         * @return Référence à this pour chaînage
         */
        template <typename Enum>
        BindingGenerator &bind_enum(const std::string                                   &name,
                                    std::initializer_list<std::pair<const char *, Enum>> values) {
            Napi::Object enum_obj = Napi::Object::New(env_);

            for (const auto &[value_name, value] : values) {
                enum_obj.Set(value_name, Napi::Number::New(env_, static_cast<int>(value)));
            }

            exports_.Set(name, enum_obj);

            return *this;
        }

        // Register a class factory when binding
        template <typename Class> void register_class_factory(const std::string &class_name) {
            std::type_index type_idx(typeid(Class));
            type_to_classname_[type_idx] = class_name;

            // Factory function that creates JS wrapper from C++ object
            class_factories_[type_idx] = [this, class_name](Napi::Env   env,
                                                            const void *cpp_ptr) -> Napi::Object {
                const Class &cpp_obj = *static_cast<const Class *>(cpp_ptr);

                auto it = class_constructors_.find(class_name);
                if (it == class_constructors_.end()) {
                    throw std::runtime_error("Constructor not found for: " + class_name);
                }

                Napi::Function ctor     = it->second.Value();
                Napi::Object   obj      = ctor.New({});
                auto          *wrapper  = ClassWrapper<Class>::Unwrap(obj);
                wrapper->get_instance() = cpp_obj;

                return obj;
            };
        }

    private:
        // ========================================================================
        // CRÉATION DE CLASSE
        // ========================================================================

        template <typename Class>
        void create_class(const std::string &name, const core::ClassMetadata<Class> &meta,
                          const core::InheritanceInfo &inheritance) {

            // Define the class with empty properties first
            Napi::Function ctor = Napi::ObjectWrap<ClassWrapper<Class>>::DefineClass(
                env_, name.c_str(), {} // Empty - we'll add properties to prototype after
            );

            // Now add properties to the prototype
            Napi::Object prototype = ctor.Get("prototype").As<Napi::Object>();

            // Add fields
            for (const auto &field_name : meta.fields()) {
                auto getter_lambda = [this, field_name,
                                      &meta](const Napi::CallbackInfo &info) -> Napi::Value {
                    Napi::Env env = info.Env();
                    try {
                        auto *wrapper = ClassWrapper<Class>::Unwrap(info.This().As<Napi::Object>());
                        Class    &instance = wrapper->get_instance();
                        core::Any value    = meta.get_field(instance, field_name);
                        return this->any_to_napi(env, value);
                    } catch (const std::exception &e) {
                        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
                        return env.Undefined();
                    }
                };

                auto setter_lambda = [this, field_name, &meta](const Napi::CallbackInfo &info) {
                    Napi::Env env = info.Env();
                    if (info.Length() < 1) {
                        Napi::TypeError::New(env, "Expected a value").ThrowAsJavaScriptException();
                        return;
                    }
                    try {
                        auto *wrapper = ClassWrapper<Class>::Unwrap(info.This().As<Napi::Object>());
                        Class    &instance = wrapper->get_instance();
                        core::Any value    = this->napi_to_any(env, info[0]);
                        meta.set_field(instance, field_name, value);
                    } catch (const std::exception &e) {
                        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
                    }
                };

                prototype.DefineProperty(
                    Napi::PropertyDescriptor::Accessor(field_name, getter_lambda, setter_lambda));
            }

            // Add methods
            for (const auto &method_name : meta.methods()) {
                auto method_lambda = [this, method_name,
                                      &meta](const Napi::CallbackInfo &info) -> Napi::Value {
                    Napi::Env env = info.Env();
                    try {
                        auto *wrapper = ClassWrapper<Class>::Unwrap(info.This().As<Napi::Object>());
                        Class &instance = wrapper->get_instance();

                        // std::vector<core::Any> args;
                        // for (size_t i = 0; i < info.Length(); ++i) {
                        //     args.push_back(this->napi_to_any(env, info[i]));
                        // }
                        auto                  &reg = this->converter_registry_;
                        std::vector<core::Any> args;
                        args.reserve(info.Length());
                        for (size_t i = 0; i < info.Length(); ++i) {
                            args.emplace_back(reg.to_any(env, info[i]));
                        }

                        core::Any result = meta.invoke_method(instance, method_name, args);
                        return this->any_to_napi(env, result);
                    } catch (const std::exception &e) {
                        Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
                        return env.Undefined();
                    }
                };

                prototype.Set(method_name, Napi::Function::New(env_, method_lambda, method_name));
            }

            // Store and export
            class_constructors_[name] = Napi::Persistent(ctor);
            exports_.Set(name, ctor);
            converter_registry_.register_class_wrapper<Class>(name);
        }

        // ========================================================================
        // WRAPPER DE CLASSE
        // ========================================================================

        template <typename Class>
        class ClassWrapper : public Napi::ObjectWrap<ClassWrapper<Class>> {
            std::shared_ptr<Class> instance_;

        public:
            static Napi::Object Init(Napi::Env env, const std::string &class_name) {
                return Napi::ObjectWrap<ClassWrapper<Class>>::DefineClass(env, class_name, {});
            }

            ClassWrapper(const Napi::CallbackInfo &info)
                : Napi::ObjectWrap<ClassWrapper<Class>>(info) {

                Napi::Env env = info.Env();

                try {
                    // Créer une instance
                    if constexpr (std::is_default_constructible_v<Class>) {
                        instance_ = std::make_shared<Class>();
                    } else {
                        // TODO: Gérer les constructeurs avec paramètres
                        Napi::Error::New(env, "Class requires constructor arguments")
                            .ThrowAsJavaScriptException();
                    }
                } catch (const std::exception &e) {
                    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
                }
            }

            Class                 &get_instance() { return *instance_; }
            const Class           &get_instance() const { return *instance_; }
            std::shared_ptr<Class> get_shared_instance() { return instance_; }

            static ClassWrapper<Class> *Unwrap(Napi::Object obj) {
                return Napi::ObjectWrap<ClassWrapper<Class>>::Unwrap(obj);
            }
        };

        // ========================================================================
        // CRÉATION DE PROPRIÉTÉS POUR CHAMPS
        // ========================================================================

        template <typename Class>
        Napi::ClassPropertyDescriptor<Class>
        create_field_property(const std::string                &field_name,
                              const core::ClassMetadata<Class> &meta) {

            // Getter
            auto getter = [this, field_name, &meta](const Napi::CallbackInfo &info) -> Napi::Value {
                Napi::Env env = info.Env();

                try {
                    auto  *wrapper  = ClassWrapper<Class>::Unwrap(info.This().As<Napi::Object>());
                    Class &instance = wrapper->get_instance();

                    core::Any value = meta.get_field(instance, field_name);
                    return this->any_to_napi(env, value);

                } catch (const std::exception &e) {
                    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
                    return env.Undefined();
                }
            };

            // Setter
            auto setter = [this, field_name, &meta](const Napi::CallbackInfo &info) {
                Napi::Env env = info.Env();

                if (info.Length() < 1) {
                    Napi::TypeError::New(env, "Expected a value").ThrowAsJavaScriptException();
                    return;
                }

                try {
                    auto  *wrapper  = ClassWrapper<Class>::Unwrap(info.This().As<Napi::Object>());
                    Class &instance = wrapper->get_instance();

                    core::Any value = this->napi_to_any(env, info[0]);
                    meta.set_field(instance, field_name, value);

                } catch (const std::exception &e) {
                    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
                }
            };

            return Napi::ObjectWrap<ClassWrapper<Class>>::InstanceAccessor(field_name, getter,
                                                                           setter);
        }

        // ========================================================================
        // CRÉATION DE PROPRIÉTÉS POUR MÉTHODES
        // ========================================================================

        template <typename Class>
        Napi::ClassPropertyDescriptor<Class>
        create_method_property(const std::string                &method_name,
                               const core::ClassMetadata<Class> &meta) {

            auto method = [this, method_name,
                           &meta](const Napi::CallbackInfo &info) -> Napi::Value {
                Napi::Env env = info.Env();

                try {
                    auto  *wrapper  = ClassWrapper<Class>::Unwrap(info.This().As<Napi::Object>());
                    Class &instance = wrapper->get_instance();

                    // Convertir les arguments
                    std::vector<core::Any> args;
                    for (size_t i = 0; i < info.Length(); ++i) {
                        args.push_back(this->napi_to_any(env, info[i]));
                    }

                    // Invoquer la méthode
                    core::Any result = meta.invoke_method(instance, method_name, args);

                    return this->any_to_napi(env, result);

                } catch (const std::exception &e) {
                    Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
                    return env.Undefined();
                }
            };

            return Napi::ObjectWrap<ClassWrapper<Class>>::InstanceMethod(method_name, method);
        }

        // ========================================================================
        // CONVERSION ANY → NAPI
        // ========================================================================

        Napi::Value any_to_napi(Napi::Env env, const core::Any &value) {
            if (!value.has_value()) {
                return env.Undefined();
            }

            // Try primitive types first
            try {
                return Napi::Number::New(env, value.as<double>());
            } catch (...) {
            }
            try {
                return Napi::Number::New(env, value.as<float>());
            } catch (...) {
            }
            try {
                return Napi::Number::New(env, value.as<int>());
            } catch (...) {
            }
            try {
                return Napi::Number::New(env, value.as<int32_t>());
            } catch (...) {
            }
            try {
                return Napi::Number::New(env, value.as<uint32_t>());
            } catch (...) {
            }
            try {
                return Napi::Number::New(env, value.as<int64_t>());
            } catch (...) {
            }
            try {
                return Napi::Boolean::New(env, value.as<bool>());
            } catch (...) {
            }
            try {
                return Napi::String::New(env, value.as<std::string>());
            } catch (...) {
            }

            std::type_index type_idx = value.get_type_index();

            // Check if it's a registered class type
            auto factory_it = this->class_factories_.find(type_idx);
            if (factory_it != this->class_factories_.end()) {
                // Use the factory to create JS wrapper
                // We need a way to get the void* to the value...
                // This requires another enhancement to Any
                const void *ptr = value.get_void_ptr(); // Need to add this
                return factory_it->second(env, ptr);
            }

            // Fall back to primitives
            try {
                return converter_registry_.to_napi(env, value.as<double>());
            } catch (...) {
            }
            try {
                return converter_registry_.to_napi(env, value.as<float>());
            } catch (...) {
            }

            // If nothing works, return undefined
            return env.Undefined();
        }

        // ========================================================================
        // CONVERSION NAPI → ANY
        // ========================================================================

        core::Any napi_to_any(Napi::Env env, Napi::Value value) {
            if (value.IsUndefined() || value.IsNull()) {
                return core::Any();
            }

            if (value.IsBoolean()) {
                return core::Any(value.As<Napi::Boolean>().Value());
            }

            if (value.IsNumber()) {
                return core::Any(value.As<Napi::Number>().DoubleValue());
            }

            if (value.IsString()) {
                return core::Any(value.As<Napi::String>().Utf8Value());
            }

            // TODO: Handle arrays, objects, etc.

            return core::Any();
        }

        // ========================================================================
        // CONVERSION DES ARGUMENTS
        // ========================================================================

        template <typename... Args, size_t... Is>
        std::tuple<Args...> convert_args(const Napi::CallbackInfo &info,
                                         std::index_sequence<Is...>) {
            return std::make_tuple(
                converter_registry_.from_napi<std::decay_t<Args>>(info.Env(), info[Is])...);
        }
    };

} // namespace rosetta::generators::js

// ============================================================================
// MACROS HELPER
// ============================================================================

/**
 * @brief Macro pour initialiser un module Node.js
 */
#define BEGIN_NAPI_MODULE(module_name) \
    Napi::Object Init_##module_name(Napi::Env env, Napi::Object exports)

/**
 * @brief Macro pour enregistrer le module
 */
#define END_NAPI_MODULE(module_name) NODE_API_MODULE(module_name, Init_##module_name)

/**
 * @brief Macro pour lier automatiquement une classe
 */
#define NAPI_AUTO_BIND_CLASS(env, exports, Class) \
    rosetta::generators::js::BindingGenerator(env, exports).bind_class<Class>()

/**
 * @brief Macro pour lier plusieurs classes
 */
#define NAPI_AUTO_BIND_CLASSES(env, exports, ...) \
    rosetta::generators::js::BindingGenerator(env, exports).bind_classes<__VA_ARGS__>()

/**
 * @brief Macro pour lier une fonction
 */
#define NAPI_BIND_FUNCTION(env, exports, func, name) \
    rosetta::generators::js::BindingGenerator(env, exports).bind_function(func, name)

/**
 * @brief Macro pour lier une énumération
 */
#define NAPI_BIND_ENUM(env, exports, EnumType, name, ...) \
    rosetta::generators::js::BindingGenerator(env, exports).bind_enum<EnumType>(name, {__VA_ARGS__})