// ============================================================================
// Emscripten binding generator using Rosetta introspection
// Simplified version compared to PyGenerator
// ============================================================================
#pragma once

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <rosetta/rosetta.h>
#include <string>
#include <typeindex>
#include <vector>

namespace em = emscripten;

namespace rosetta::em_gen {

    // ============================================================================
    // Type Converters - Convert between C++ Any and JavaScript val
    // ============================================================================

    /**
     * @brief Convert C++ Any to JavaScript val
     */
    inline em::val any_to_val(const core::Any &value) {
        if (!value.has_value()) {
            return em::val::undefined();
        }

        auto type = value.get_type_index();

        // Primitives
        if (type == std::type_index(typeid(int))) {
            return em::val(value.as<int>());
        }
        if (type == std::type_index(typeid(double))) {
            return em::val(value.as<double>());
        }
        if (type == std::type_index(typeid(float))) {
            return em::val(value.as<float>());
        }
        if (type == std::type_index(typeid(bool))) {
            return em::val(value.as<bool>());
        }
        if (type == std::type_index(typeid(std::string))) {
            return em::val(value.as<std::string>());
        }
        if (type == std::type_index(typeid(size_t))) {
            return em::val(static_cast<unsigned long>(value.as<size_t>()));
        }
        if (type == std::type_index(typeid(long))) {
            return em::val(value.as<long>());
        }

        // Void return
        if (type == std::type_index(typeid(void))) {
            return em::val::undefined();
        }

        // Vector types
        if (type == std::type_index(typeid(std::vector<double>))) {
            const auto &vec = value.as<std::vector<double>>();
            em::val arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i) {
                arr.set(i, vec[i]);
            }
            return arr;
        }
        if (type == std::type_index(typeid(std::vector<int>))) {
            const auto &vec = value.as<std::vector<int>>();
            em::val arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i) {
                arr.set(i, vec[i]);
            }
            return arr;
        }
        if (type == std::type_index(typeid(std::vector<std::string>))) {
            const auto &vec = value.as<std::vector<std::string>>();
            em::val arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i) {
                arr.set(i, vec[i]);
            }
            return arr;
        }

        // Unknown type - return undefined
        return em::val::undefined();
    }

    /**
     * @brief Convert JavaScript val to C++ Any based on expected type
     */
    inline core::Any val_to_any(const em::val &js_val, std::type_index expected_type) {
        // Primitives
        if (expected_type == std::type_index(typeid(int))) {
            return core::Any(js_val.as<int>());
        }
        if (expected_type == std::type_index(typeid(double))) {
            return core::Any(js_val.as<double>());
        }
        if (expected_type == std::type_index(typeid(float))) {
            return core::Any(js_val.as<float>());
        }
        if (expected_type == std::type_index(typeid(bool))) {
            return core::Any(js_val.as<bool>());
        }
        if (expected_type == std::type_index(typeid(std::string))) {
            return core::Any(js_val.as<std::string>());
        }
        if (expected_type == std::type_index(typeid(size_t))) {
            return core::Any(static_cast<size_t>(js_val.as<unsigned long>()));
        }

        // Vector types
        if (expected_type == std::type_index(typeid(std::vector<double>))) {
            std::vector<double> vec;
            unsigned length = js_val["length"].as<unsigned>();
            for (unsigned i = 0; i < length; ++i) {
                vec.push_back(js_val[i].as<double>());
            }
            return core::Any(vec);
        }
        if (expected_type == std::type_index(typeid(std::vector<int>))) {
            std::vector<int> vec;
            unsigned length = js_val["length"].as<unsigned>();
            for (unsigned i = 0; i < length; ++i) {
                vec.push_back(js_val[i].as<int>());
            }
            return core::Any(vec);
        }
        if (expected_type == std::type_index(typeid(std::vector<std::string>))) {
            std::vector<std::string> vec;
            unsigned length = js_val["length"].as<unsigned>();
            for (unsigned i = 0; i < length; ++i) {
                vec.push_back(js_val[i].as<std::string>());
            }
            return core::Any(vec);
        }

        throw std::runtime_error("Unsupported type for JavaScript conversion");
    }

    // ============================================================================
    // Emscripten Class Binder
    // ============================================================================

    /**
     * @brief Helper to bind a class using Rosetta metadata
     */
    template <typename T>
    class EmClassBinder {
    public:
        /**
         * @brief Bind constructors from metadata
         */
        template <typename ClassType>
        static void bind_constructors(ClassType &em_class, const core::ClassMetadata<T> &meta) {
            const auto &ctors = meta.constructor_infos();
            
            // Bind default constructor if available
            for (const auto &ctor : ctors) {
                if (ctor.arity == 0) {
                    em_class.template constructor<>();
                    break; // Only bind one default constructor
                }
            }
            // Note: Emscripten has limited support for multiple constructors
            // For complex cases, use factory functions
        }

        /**
         * @brief Bind fields/properties from metadata
         * 
         * Note: Emscripten's embind requires function pointers for property getters/setters,
         * and doesn't support lambdas with captures. For fields, users should either:
         * 1. Bind them manually using .property("name", &Class::member)
         * 2. Use the getter/setter methods if they're registered
         * 
         * The methods registered via Rosetta (like getName/setName) will still be bound
         * automatically by bind_methods().
         */
        template <typename ClassType>
        static void bind_fields(ClassType &em_class, const core::ClassMetadata<T> &meta) {
            // Fields require compile-time member pointer info that we don't have at runtime.
            // The Rosetta introspection stores type-erased getters/setters which can't be
            // converted to the function pointers Emscripten needs.
            // 
            // Recommendation: Register getter/setter methods in Rosetta metadata instead:
            //   .method("getX", &MyClass::getX)
            //   .method("setX", &MyClass::setX)
            // These will be automatically bound by bind_methods().
        }

        /**
         * @brief Bind methods from metadata
         * 
         * Note: Emscripten's embind requires function pointers and doesn't support
         * lambdas with captures. The Rosetta introspection uses type-erased invokers
         * that cannot be converted to what embind needs.
         * 
         * For automatic method binding to work with Emscripten, methods must be
         * bound explicitly using the standard embind syntax:
         *   .function("methodName", &Class::methodName)
         * 
         * This generator provides utilities and class structure, but method binding
         * should be done explicitly or via a code generation approach.
         */
        template <typename ClassType>
        static void bind_methods(ClassType &em_class, const core::ClassMetadata<T> &meta) {
            // Emscripten embind requires compile-time function pointer information.
            // Rosetta's runtime introspection stores type-erased invokers which
            // cannot be converted to the function pointers embind needs.
            //
            // Options for users:
            // 1. Bind methods manually: .function("greet", &Person::greet)
            // 2. Use a code generator that reads Rosetta metadata and generates embind code
            // 3. Use the template-based approach shown in bind_class_with_methods()
        }
    };

    // ============================================================================
    // Emscripten Generator
    // ============================================================================

    /**
     * @brief Generator for Emscripten bindings
     * 
     * Usage:
     *   EMSCRIPTEN_BINDINGS(my_module) {
     *       EmGenerator gen;
     *       gen.bind_class<MyClass>("MyClass");
     *   }
     */
    class EmGenerator {
    public:
        EmGenerator() = default;

        /**
         * @brief Bind a class that's registered with Rosetta
         * @tparam T The C++ class type
         * @param js_name JavaScript class name
         */
        template <typename T>
        EmGenerator &bind_class(const std::string &js_name) {
            // Get metadata from registry
            const auto &meta = core::Registry::instance().get<T>();
            std::string final_name = js_name.empty() ? meta.name() : js_name;

            // Create the emscripten class binding
            auto em_class = em::class_<T>(final_name.c_str());

            // Bind constructors
            if constexpr (!std::is_abstract_v<T>) {
                EmClassBinder<T>::bind_constructors(em_class, meta);
            }

            // Bind fields
            EmClassBinder<T>::bind_fields(em_class, meta);

            // Bind methods
            EmClassBinder<T>::bind_methods(em_class, meta);

            return *this;
        }

        /**
         * @brief Bind a free function
         */
        template <typename Ret, typename... Args>
        EmGenerator &bind_function(const std::string &name, Ret (*func)(Args...)) {
            em::function(name.c_str(), func);
            return *this;
        }

        /**
         * @brief Add utility functions
         */
        EmGenerator &add_utilities() {
            // Helper function for listClasses
            struct Utilities {
                static em::val listClasses() {
                    auto classes = core::Registry::instance().list_classes();
                    em::val arr = em::val::array();
                    for (size_t i = 0; i < classes.size(); ++i) {
                        arr.set(i, classes[i]);
                    }
                    return arr;
                }
                
                static std::string version() {
                    return rosetta::version();
                }
            };

            em::function("listClasses", &Utilities::listClasses);
            em::function("version", &Utilities::version);

            return *this;
        }
    };

    /**
     * @brief Convenience function to create an EmGenerator
     */
    inline EmGenerator create_bindings() {
        return EmGenerator();
    }

} // namespace rosetta::em_gen

// ============================================================================
// Helper Macros for Manual Binding (Recommended Approach)
// ============================================================================

/**
 * Since Emscripten requires compile-time function pointers, the recommended
 * approach is to use these helper macros that combine Rosetta metadata
 * with explicit embind calls.
 */

/**
 * @brief Begin an Emscripten class binding
 */
#define EM_BEGIN_CLASS(Class) \
    emscripten::class_<Class>(#Class)

/**
 * @brief Bind a default constructor
 */
#define EM_CONSTRUCTOR() \
    .constructor<>()

/**
 * @brief Bind a constructor with specific argument types
 */
#define EM_CONSTRUCTOR_ARGS(...) \
    .constructor<__VA_ARGS__>()

/**
 * @brief Bind a property (public member variable)
 */
#define EM_PROPERTY(Class, member) \
    .property(#member, &Class::member)

/**
 * @brief Bind a method
 */
#define EM_METHOD(Class, method) \
    .function(#method, &Class::method)

/**
 * @brief Bind a method with a custom name
 */
#define EM_METHOD_AS(name, Class, method) \
    .function(name, &Class::method)

/**
 * @brief Bind a static function
 */
#define EM_STATIC_METHOD(Class, method) \
    .class_function(#method, &Class::method)

/**
 * @brief Simplified macro for Emscripten module with generator
 * 
 * Usage:
 *   BEGIN_EM_MODULE(mymodule) {
 *       gen.bind_class<Person>("Person");
 *       gen.add_utilities();
 *   }
 *   END_EM_MODULE();
 */
#define BEGIN_EM_MODULE(module_name) \
    EMSCRIPTEN_BINDINGS(module_name) { \
        auto gen = rosetta::em_gen::create_bindings();

#define BIND_EM_CLASS(Class) gen.bind_class<Class>(#Class);

#define BIND_EM_FUNCTION(func) gen.bind_function(#func, func);

#define BIND_EM_UTILITIES() gen.add_utilities();

#define END_EM_MODULE() }

// ============================================================================
// Example Usage with Manual Binding (Recommended)
// ============================================================================
/*
EMSCRIPTEN_BINDINGS(my_module) {
    // Use the helper macros for clean syntax
    EM_BEGIN_CLASS(Person)
        EM_CONSTRUCTOR()
        EM_CONSTRUCTOR_ARGS(std::string, int)
        EM_PROPERTY(Person, name)
        EM_PROPERTY(Person, age)
        EM_METHOD(Person, greet)
        EM_METHOD(Person, haveBirthday)
    ;
    
    // Add Rosetta utilities
    rosetta::em_gen::EmGenerator().add_utilities();
}
*/