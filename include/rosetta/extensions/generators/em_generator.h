// ============================================================================
// Emscripten binding generator using Rosetta introspection
// Version 3: Clean JavaScript API with automatic binding
// ============================================================================
#pragma once

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <rosetta/rosetta.h>
#include <string>
#include <typeindex>
#include <vector>
#include <sstream>

namespace em = emscripten;

namespace rosetta::em_gen {

    // ============================================================================
    // Type Cast Registry - For converting Any back to em::val
    // ============================================================================

    class TypeCastRegistry {
    public:
        using CastFunc = std::function<em::val(const core::Any&)>;

        static TypeCastRegistry& instance() {
            static TypeCastRegistry reg;
            return reg;
        }

        template <typename T>
        void register_cast() {
            casts_[std::type_index(typeid(T))] = [](const core::Any& value) -> em::val {
                return em::val(value.as<T>());
            };
        }

        em::val cast(const core::Any& value) const {
            auto it = casts_.find(value.get_type_index());
            if (it != casts_.end()) {
                return it->second(value);
            }
            return em::val::undefined();
        }

        bool has_cast(std::type_index type) const {
            return casts_.find(type) != casts_.end();
        }

    private:
        std::unordered_map<std::type_index, CastFunc> casts_;
    };

    // ============================================================================
    // Type Converters
    // ============================================================================

    inline em::val any_to_val(const core::Any &value) {
        if (!value.has_value()) return em::val::undefined();

        auto type = value.get_type_index();

        // Primitives
        if (type == std::type_index(typeid(int))) return em::val(value.as<int>());
        if (type == std::type_index(typeid(double))) return em::val(value.as<double>());
        if (type == std::type_index(typeid(float))) return em::val(value.as<float>());
        if (type == std::type_index(typeid(bool))) return em::val(value.as<bool>());
        if (type == std::type_index(typeid(std::string))) return em::val(value.as<std::string>());
        if (type == std::type_index(typeid(size_t))) return em::val(static_cast<unsigned long>(value.as<size_t>()));
        if (type == std::type_index(typeid(long))) return em::val(value.as<long>());
        if (type == std::type_index(typeid(void))) return em::val::undefined();

        // Vectors
        if (type == std::type_index(typeid(std::vector<double>))) {
            const auto &vec = value.as<std::vector<double>>();
            em::val arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i) arr.set(i, vec[i]);
            return arr;
        }
        if (type == std::type_index(typeid(std::vector<int>))) {
            const auto &vec = value.as<std::vector<int>>();
            em::val arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i) arr.set(i, vec[i]);
            return arr;
        }
        if (type == std::type_index(typeid(std::vector<std::string>))) {
            const auto &vec = value.as<std::vector<std::string>>();
            em::val arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i) arr.set(i, vec[i]);
            return arr;
        }

        // Check registered type casts
        if (TypeCastRegistry::instance().has_cast(type)) {
            return TypeCastRegistry::instance().cast(value);
        }

        return em::val::undefined();
    }

    inline core::Any val_to_any(const em::val &js_val, std::type_index expected_type) {
        if (expected_type == std::type_index(typeid(int))) return core::Any(js_val.as<int>());
        if (expected_type == std::type_index(typeid(double))) return core::Any(js_val.as<double>());
        if (expected_type == std::type_index(typeid(float))) return core::Any(js_val.as<float>());
        if (expected_type == std::type_index(typeid(bool))) return core::Any(js_val.as<bool>());
        if (expected_type == std::type_index(typeid(std::string))) return core::Any(js_val.as<std::string>());
        if (expected_type == std::type_index(typeid(size_t))) return core::Any(static_cast<size_t>(js_val.as<unsigned long>()));

        // Vectors
        if (expected_type == std::type_index(typeid(std::vector<double>))) {
            std::vector<double> vec;
            unsigned length = js_val["length"].as<unsigned>();
            for (unsigned i = 0; i < length; ++i) vec.push_back(js_val[i].as<double>());
            return core::Any(vec);
        }
        if (expected_type == std::type_index(typeid(std::vector<int>))) {
            std::vector<int> vec;
            unsigned length = js_val["length"].as<unsigned>();
            for (unsigned i = 0; i < length; ++i) vec.push_back(js_val[i].as<int>());
            return core::Any(vec);
        }

        throw std::runtime_error("Unsupported type for JavaScript conversion");
    }

    // ============================================================================
    // Constructor Binder
    // ============================================================================

    template <typename T, typename TupleType>
    struct ConstructorBinder;

    template <typename T, typename... Args>
    struct ConstructorBinder<T, std::tuple<Args...>> {
        template <typename ClassType>
        static void bind(ClassType &em_class) {
            em_class.template constructor<Args...>();
        }
    };

    // ============================================================================
    // Class Wrapper - Provides generic access via _get/_set/_call
    // ============================================================================

    template <typename T>
    struct ClassWrapper {
        // Field getter
        static em::val getField(T& obj, const std::string& name) {
            const auto &meta = core::Registry::instance().get<T>();
            try {
                core::Any value = meta.get_field(obj, name);
                return any_to_val(value);
            } catch (const std::exception& e) {
                throw std::runtime_error("Field '" + name + "' not found: " + e.what());
            }
        }

        // Field setter
        static void setField(T& obj, const std::string& name, em::val value) {
            const auto &meta = core::Registry::instance().get<T>();
            try {
                auto field_type = meta.get_field_type(name);
                core::Any cpp_val = val_to_any(value, field_type);
                meta.set_field(obj, name, cpp_val);
            } catch (const std::exception& e) {
                throw std::runtime_error("Cannot set field '" + name + "': " + e.what());
            }
        }

        // Method caller
        static em::val callMethod(T& obj, const std::string& name, em::val args) {
            const auto &meta = core::Registry::instance().get<T>();
            try {
                size_t arity = meta.get_method_arity(name);
                const auto &arg_types = meta.get_method_arg_types(name);

                std::vector<core::Any> cpp_args;
                for (size_t i = 0; i < arity; ++i) {
                    cpp_args.push_back(val_to_any(args[i], arg_types[i]));
                }

                core::Any result = meta.invoke_method(obj, name, cpp_args);
                return any_to_val(result);
            } catch (const std::exception& e) {
                throw std::runtime_error("Method '" + name + "' call failed: " + e.what());
            }
        }

        // Get class metadata as JSON
        static em::val getMetadata() {
            const auto &meta = core::Registry::instance().get<T>();
            em::val info = em::val::object();

            // Fields
            em::val fields = em::val::array();
            for (const auto& f : meta.fields()) {
                fields.call<void>("push", em::val(f));
            }
            info.set("fields", fields);

            // Methods
            em::val methods = em::val::array();
            for (const auto& m : meta.methods()) {
                methods.call<void>("push", em::val(m));
            }
            info.set("methods", methods);

            return info;
        }
    };

    // ============================================================================
    // Auto Class Binder
    // ============================================================================

    template <typename T, typename... CtorSignatures>
    class AutoBinder {
    public:
        static void bind(const std::string& js_name) {
            const auto &meta = core::Registry::instance().get<T>();
            std::string name = js_name.empty() ? meta.name() : js_name;

            // Register type cast for this class so methods returning T work
            TypeCastRegistry::instance().register_cast<T>();

            auto em_class = em::class_<T>(name.c_str());

            // Bind constructors
            if constexpr (!std::is_abstract_v<T>) {
                if constexpr (sizeof...(CtorSignatures) > 0) {
                    (ConstructorBinder<T, CtorSignatures>::bind(em_class), ...);
                } else {
                    // Try default constructor
                    const auto &ctors = meta.constructor_infos();
                    for (const auto &ctor : ctors) {
                        if (ctor.arity == 0) {
                            em_class.template constructor<>();
                            break;
                        }
                    }
                }
            }

            // Bind generic accessors
            em_class.function("$get", &ClassWrapper<T>::getField);
            em_class.function("$set", &ClassWrapper<T>::setField);
            em_class.function("$call", &ClassWrapper<T>::callMethod);
            em_class.class_function("$meta", &ClassWrapper<T>::getMetadata);
        }
    };

    // ============================================================================
    // Generator
    // ============================================================================

    class EmGenerator {
    public:
        EmGenerator() = default;

        template <typename T, typename... CtorSignatures>
        EmGenerator &bind_class(const std::string &js_name = "") {
            AutoBinder<T, CtorSignatures...>::bind(js_name);
            return *this;
        }

        template <typename Ret, typename... Args>
        EmGenerator &bind_function(const std::string &name, Ret (*func)(Args...)) {
            em::function(name.c_str(), func);
            return *this;
        }

        EmGenerator &add_utilities() {
            struct Utils {
                static em::val listClasses() {
                    auto classes = core::Registry::instance().list_classes();
                    em::val arr = em::val::array();
                    for (size_t i = 0; i < classes.size(); ++i) {
                        arr.set(i, classes[i]);
                    }
                    return arr;
                }
                static std::string version() { return rosetta::version(); }
            };
            em::function("listClasses", &Utils::listClasses);
            em::function("version", &Utils::version);
            return *this;
        }
    };

    inline EmGenerator create_bindings() { return EmGenerator(); }

    // ============================================================================
    // JavaScript Enhancement Code Generator
    // ============================================================================

    /**
     * @brief Generate JavaScript code that enhances classes with proper getters/setters
     * 
     * Call this and execute the returned string in JavaScript after module loads
     */
    inline std::string generateJSEnhancements() {
        std::ostringstream js;
        
        js << R"(
function enhanceRosettaClasses(Module) {
    const classes = Module.listClasses();
    
    classes.forEach(className => {
        const ClassRef = Module[className];
        if (!ClassRef) return;
        
        // Get metadata
        const meta = ClassRef.$meta();
        
        // Add field accessors as properties
        meta.fields.forEach(field => {
            Object.defineProperty(ClassRef.prototype, field, {
                get: function() { return this.$get(field); },
                set: function(v) { this.$set(field, v); },
                enumerable: true
            });
        });
        
        // Add method wrappers
        meta.methods.forEach(method => {
            if (!ClassRef.prototype[method]) {
                ClassRef.prototype[method] = function(...args) {
                    return this.$call(method, args);
                };
            }
        });
    });
    
    return Module;
}
)";
        return js.str();
    }

} // namespace rosetta::em_gen

// ============================================================================
// Macros
// ============================================================================

#define BEGIN_EM_MODULE(module_name) \
    EMSCRIPTEN_BINDINGS(module_name) { \
        auto gen = rosetta::em_gen::create_bindings();

#define BIND_EM_CLASS_AUTO(Class, ...) \
    gen.bind_class<Class, __VA_ARGS__>(#Class);

#define BIND_EM_CLASS(Class) \
    gen.bind_class<Class>(#Class);

#define BIND_EM_FUNCTION(func) gen.bind_function(#func, func);

#define BIND_EM_UTILITIES() gen.add_utilities();

#define END_EM_MODULE() }