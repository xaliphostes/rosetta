// ============================================================================
// rosetta/generators/napi_binding_generator.hpp
//
// Générateur de bindings JavaScript avec Node-API (C++ wrapper)
// ============================================================================
#pragma once
#include "../core/registry.h"
#include "javascript_type_converter_registry.h"
#include <cmath>
#include <napi.h>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace rosetta::generators {

    /**
     * @brief Générateur de bindings JavaScript utilisant node-addon-api
     *
     * Usage similaire à PythonBindingGenerator mais pour Node.js
     */
    class NapiBindingGenerator {
        Napi::Env    env_;
        Napi::Object exports_;

        // Cache des constructeurs de classes
        std::unordered_map<std::string, Napi::FunctionReference> class_constructors_;

    public:
        /**
         * @brief Constructeur
         * @param env Environnement Node-API
         * @param exports Objet exports du module
         */
        NapiBindingGenerator(Napi::Env env, Napi::Object exports) : env_(env), exports_(exports) {}

        /**
         * @brief Destructeur - les FunctionReference se nettoient automatiquement
         */
        ~NapiBindingGenerator() = default;

        // ========================================================================
        // BIND CLASS
        // ========================================================================

        /**
         * @brief Lie une classe introspectée automatiquement
         * @tparam Class Type de la classe
         * @param custom_name Nom custom (optionnel)
         */
        template <typename Class>
        NapiBindingGenerator &bind_class(const std::string &custom_name = "") {
            auto &registry = core::Registry::instance();

            if (!registry.has_class<Class>()) {
                throw std::runtime_error("Class not registered in Rosetta: " +
                                         std::string(typeid(Class).name()));
            }

            const auto &meta        = registry.get<Class>();
            const auto &inheritance = meta.inheritance();

            std::string class_name = custom_name.empty() ? meta.name() : custom_name;

            // Créer la classe JavaScript
            create_class<Class>(class_name, meta, inheritance);

            return *this;
        }

        // ========================================================================
        // BIND MULTIPLE CLASSES
        // ========================================================================

        /**
         * @brief Lie plusieurs classes en une seule ligne
         */
        template <typename... Classes> NapiBindingGenerator &bind_classes() {
            (bind_class<Classes>(), ...);
            return *this;
        }

        // ========================================================================
        // BIND FUNCTION
        // ========================================================================

        /**
         * @brief Lie une fonction libre
         * @tparam Ret Type de retour
         * @tparam Args Types des arguments
         * @param func Pointeur vers la fonction
         * @param name Nom en JavaScript
         */
        template <typename Ret, typename... Args>
        NapiBindingGenerator &bind_function(Ret (*func)(Args...), const std::string &name) {
            // Wrapper pour la fonction
            auto callback = [func](const Napi::CallbackInfo &info) -> Napi::Value {
                Napi::Env env = info.Env();

                // TODO: Extraire les arguments, appeler la fonction, retourner le résultat
                // Pour l'instant, retourner undefined
                return env.Undefined();
            };

            // Créer la fonction JavaScript
            Napi::Function js_function = Napi::Function::New(env_, callback, name);

            // Ajouter aux exports
            exports_.Set(name, js_function);

            return *this;
        }

        // ========================================================================
        // BIND ENUM
        // ========================================================================

        /**
         * @brief Lie une énumération
         * @tparam Enum Type de l'énumération
         * @param name Nom en JavaScript
         * @param values Paires {nom, valeur}
         */
        template <typename Enum>
        NapiBindingGenerator &
        bind_enum(const std::string                                   &name,
                  std::initializer_list<std::pair<const char *, Enum>> values) {
            static_assert(std::is_enum_v<Enum>, "Type must be an enum");

            // Créer un objet JavaScript pour l'enum
            Napi::Object enum_obj = Napi::Object::New(env_);

            for (const auto &[value_name, value] : values) {
                enum_obj.Set(value_name, Napi::Number::New(env_, static_cast<int32_t>(value)));
            }

            // Ajouter aux exports
            exports_.Set(name, enum_obj);

            return *this;
        }

    private:
        // ========================================================================
        // HELPERS INTERNES
        // ========================================================================

        template <typename Class>
        void create_class(const std::string &name, const core::ClassMetadata<Class> &meta,
                          const core::InheritanceInfo &inheritance) {

            std::vector<ClassPropertyDescriptor<Class>> properties;

            // Ajouter les champs comme propriétés
            for (const auto &field_name : meta.fields()) {
                properties.push_back({field_name, meta, true});
            }

            // Ajouter les méthodes
            for (const auto &method_name : meta.methods()) {
                properties.push_back({method_name, meta, false});
            }

            // Créer le constructeur avec propriétés
            Napi::Function constructor = DefineClass<Class>(env_, name, properties);

            // Stocker une référence au constructeur
            class_constructors_[name] = Napi::Persistent(constructor);

            // Ajouter aux exports
            exports_.Set(name, constructor);
        }

        // Structure pour décrire une propriété
        template <typename Class> struct ClassPropertyDescriptor {
            std::string                       name;
            const core::ClassMetadata<Class> &meta;
            bool                              is_field; // true = field, false = method
        };

        // Définir une classe avec ses propriétés
        template <typename Class>
        Napi::Function DefineClass(Napi::Env env, const std::string &name,
                                   const std::vector<ClassPropertyDescriptor<Class>> &props) {

            std::vector<Napi::ClassPropertyDescriptor<Class>> descriptors;

            for (const auto &prop : props) {
                if (prop.is_field) {
                    // Créer un accesseur pour le champ
                    descriptors.push_back(Napi::ObjectWrap<ClassWrapper<Class>>::InstanceAccessor(
                        prop.name.c_str(), &NapiBindingGenerator::field_getter<Class>,
                        &NapiBindingGenerator::field_setter<Class>, napi_default,
                        new FieldData<Class>{prop.meta, prop.name}));
                } else {
                    // Créer une méthode
                    descriptors.push_back(Napi::ObjectWrap<ClassWrapper<Class>>::InstanceMethod(
                        prop.name.c_str(), &NapiBindingGenerator::method_wrapper<Class>,
                        napi_default, new MethodData<Class>{prop.meta, prop.name}));
                }
            }

            return Napi::ObjectWrap<ClassWrapper<Class>>::DefineClass(
                env, name.c_str(), descriptors, new ClassData<Class>{props[0].meta} // Pass metadata
            );
        }

        // Wrapper pour les classes C++
        template <typename Class>
        class ClassWrapper : public Napi::ObjectWrap<ClassWrapper<Class>> {
        public:
            Class instance_;

            ClassWrapper(const Napi::CallbackInfo &info)
                : Napi::ObjectWrap<ClassWrapper<Class>>(info), instance_() {
                // Constructeur par défaut
            }

            static Napi::Object NewInstance(Napi::Env env, const Napi::Function &constructor) {
                return constructor.New({});
            }

            Class &GetInstance() { return instance_; }
        };

        // Données pour les champs
        template <typename Class> struct FieldData {
            const core::ClassMetadata<Class> &meta;
            std::string                       name;
        };

        // Données pour les méthodes
        template <typename Class> struct MethodData {
            const core::ClassMetadata<Class> &meta;
            std::string                       name;
        };

        // Données pour la classe
        template <typename Class> struct ClassData {
            const core::ClassMetadata<Class> &meta;
        };

        // Getter pour les champs
        template <typename Class> static Napi::Value field_getter(const Napi::CallbackInfo &info) {
            Napi::Env env = info.Env();

            auto *wrapper =
                Napi::ObjectWrap<ClassWrapper<Class>>::Unwrap(info.This().As<Napi::Object>());
            auto *data = static_cast<FieldData<Class> *>(info.Data());

            // Récupérer la valeur du champ
            auto value = data->meta.get_field(wrapper->instance_, data->name);

            // Convertir en JavaScript
            return any_to_napi(env, value);
        }

        // Setter pour les champs
        template <typename Class>
        static void field_setter(const Napi::CallbackInfo &info, const Napi::Value &value) {
            auto *wrapper =
                Napi::ObjectWrap<ClassWrapper<Class>>::Unwrap(info.This().As<Napi::Object>());
            auto *data = static_cast<FieldData<Class> *>(info.Data());

            // Convertir la valeur JavaScript en C++
            auto cpp_value = napi_to_any(info.Env(), value);

            // Définir le champ
            data->meta.set_field(wrapper->instance_, data->name, cpp_value);
        }

        // Wrapper pour les méthodes
        template <typename Class>
        static Napi::Value method_wrapper(const Napi::CallbackInfo &info) {
            Napi::Env env = info.Env();

            auto *wrapper =
                Napi::ObjectWrap<ClassWrapper<Class>>::Unwrap(info.This().As<Napi::Object>());
            auto *data = static_cast<MethodData<Class> *>(info.Data());

            // Convertir les arguments
            std::vector<core::Any> cpp_args;
            for (size_t i = 0; i < info.Length(); ++i) {
                cpp_args.push_back(napi_to_any(env, info[i]));
            }

            // Invoquer la méthode
            auto result = data->meta.invoke_method(wrapper->instance_, data->name, cpp_args);

            // Convertir le résultat
            return any_to_napi(env, result);
        };
        
        // Stocker les infos de la méthode
        auto* method_info = new MethodInfo<Class>{&meta, method_name};
        
        return napi_property_descriptor{
            method_name.c_str(),
            nullptr,
            method,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            method_info
        };
    }
    
    // Structures pour stocker les infos
    template<typename Class>
    struct FieldInfo {
        const core::ClassMetadata<Class>* meta;
        std::string name;
    };
    
    template<typename Class>
    struct MethodInfo {
        const core::ClassMetadata<Class>* meta;
        std::string name;
    };
    
    // Conversion Any → NAPI
    static napi_value any_to_napi(napi_env env, const core::Any& value) {
        // Simplification - nécessite inspection du type réel
        if (!value.has_value()) {
            napi_value result;
            napi_get_undefined(env, &result);
            return result;
        }
        
        // Essayer différents types
        // Note: Ceci est une simplification - une vraie implémentation
        // nécessiterait un système de type registry
        
        napi_value result;
        
        try {
            // Essayer double
            double d = const_cast<core::Any&>(value).as<double>();
            napi_create_double(env, d, &result);
            return result;
        } catch (...) {}
        
        try {
            // Essayer int
            int i = const_cast<core::Any&>(value).as<int>();
            napi_create_int32(env, i, &result);
            return result;
        } catch (...) {}
        
        try {
            // Essayer bool
            bool b = const_cast<core::Any&>(value).as<bool>();
            napi_get_boolean(env, b, &result);
            return result;
        } catch (...) {}
        
        try {
            // Essayer string
            std::string s = const_cast<core::Any&>(value).as<std::string>();
            napi_create_string_utf8(env, s.c_str(), s.length(), &result);
            return result;
        } catch (...) {}
        
        // Par défaut, retourner undefined
        napi_get_undefined(env, &result);
        return result;
    }
    
    // Conversion NAPI → Any
    static core::Any napi_to_any(napi_env env, napi_value value) {
        napi_valuetype type;
        napi_typeof(env, value, &type);
        
        switch (type) {
            case napi_number: {
                double num;
                napi_get_value_double(env, value, &num);
                return core::Any(num);
            }
            case napi_string: {
                size_t length;
                napi_get_value_string_utf8(env, value, nullptr, 0, &length);
                std::string str(length, '\0');
                napi_get_value_string_utf8(env, value, &str[0], length + 1, nullptr);
                return core::Any(str);
            }
            case napi_boolean: {
                bool b;
                napi_get_value_bool(env, value, &b);
                return core::Any(b);
            }
            default:
                return core::Any(0);
        }

        // Conversion Any → Napi::Value
        static Napi::Value any_to_napi(Napi::Env env, const core::Any &value) {
            return TypeConverterRegistry::instance().any_to_napi(env, value);
        }

        // Conversion Napi::Value → Any
        static core::Any napi_to_any(Napi::Env env, const Napi::Value &value) {
            return TypeConverterRegistry::instance().napi_to_any(env, value);
        }
    };

} // namespace rosetta::generators

// ============================================================================
// MACROS HELPER
// ============================================================================

/**
 * @brief Macro pour initialiser un module Node.js
 */
#define BEGIN_MODULE(module_name) \
    Napi::Object Init_##module_name(Napi::Env env, Napi::Object exports)

/**
 * @brief Macro pour enregistrer le module
 */
#define END_MODULE(module_name) NODE_API_MODULE(module_name, Init_##module_name)

/**
 * @brief Macro pour lier automatiquement une classe
 */
#define NAPI_AUTO_BIND_CLASS(env, exports, Class) \
    rosetta::generators::NapiBindingGenerator(env, exports).bind_class<Class>()

/**
 * @brief Macro pour lier plusieurs classes
 */
#define NAPI_AUTO_BIND_CLASSES(env, exports, ...) \
    rosetta::generators::NapiBindingGenerator(env, exports).bind_classes<__VA_ARGS__>()

// ============================================================================
// EXEMPLE D'UTILISATION COMPLET
// ============================================================================

/*

#include <napi.h>
#include "rosetta/rosetta.hpp"
#include "rosetta/generators/napi_binding_generator.hpp"

// 1. Vos classes (aucune modification)
class Vector3D {
public:
    double x, y, z;

    Vector3D(double x = 0, double y = 0, double z = 0)
        : x(x), y(y), z(z) {}

    double length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    void normalize() {
        double len = length();
        if (len > 0) {
            x /= len; y /= len; z /= len;
        }
    }
};

class Point2D {
public:
    double x, y;

    Point2D(double x = 0, double y = 0) : x(x), y(y) {}

    double distance() const {
        return std::sqrt(x*x + y*y);
    }
};

enum class Status {
    Active,
    Inactive,
    Pending
};

// 2. Enregistrement Rosetta
void register_types() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);

    ROSETTA_REGISTER_CLASS(Point2D)
        .field("x", &Point2D::x)
        .field("y", &Point2D::y)
        .method("distance", &Point2D::distance);
}

// 3. Module Node.js avec binding automatique
NAPI_MODULE_INIT(geometry) {
    // Enregistrer les types d'abord
    register_types();

    // Créer le générateur
    rosetta::generators::NapiBindingGenerator generator(env, exports);

    // Lier les classes automatiquement
    generator.bind_class<Vector3D>();
    generator.bind_class<Point2D>("Point");  // Nom custom

    // Ou lier plusieurs à la fois
    // generator.bind_classes<Vector3D, Point2D>();

    // Lier une enum
    generator.bind_enum<Status>("Status", {
        {"Active", Status::Active},
        {"Inactive", Status::Inactive},
        {"Pending", Status::Pending}
    });

    return exports;
}

NAPI_MODULE_REGISTER(geometry)

// 4. Fichier binding.gyp pour compilation
{
  "targets": [
    {
      "target_name": "geometry",
      "sources": [ "bindings.cpp" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "path/to/rosetta"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_CPP_EXCEPTIONS" ],
      "cflags_cc": [ "-std=c++17" ]
    }
  ]
}

// 5. Compilation
// $ npm install node-addon-api
// $ node-gyp configure
// $ node-gyp build

// 6. Utilisation en Node.js
const geometry = require('./build/Release/geometry');

const v = new geometry.Vector3D();
v.x = 3.0;
v.y = 4.0;
v.z = 0.0;

console.log(v.length());  // 5.0
v.normalize();
console.log(v.x, v.y, v.z);  // 0.6, 0.8, 0.0

const p = new geometry.Point();
p.x = 3;
p.y = 4;
console.log(p.distance());  // 5.0

console.log(geometry.Status.Active);  // 0

*/