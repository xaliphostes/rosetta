// ============================================================================
// Emscripten binding generator using Rosetta introspection
// Version 5: Auto-bind inherited methods (no JS enhancement needed)
// ============================================================================
#pragma once

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <functional>
#include <map>
#include <rosetta/rosetta.h>
#include <set>
#include <sstream>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace em = emscripten;

namespace rosetta::em_gen {

    // ============================================================================
    // Type Cast Registry - For converting Any back to em::val
    // ============================================================================

    class TypeCastRegistry {
    public:
        using CastFunc = std::function<em::val(const core::Any &)>;

        static TypeCastRegistry &instance() {
            static TypeCastRegistry reg;
            return reg;
        }

        template <typename T> void register_cast() {
            // Don't register casts for abstract classes - they can't be instantiated
            if constexpr (!std::is_abstract_v<T>) {
                casts_[std::type_index(typeid(T))] = [](const core::Any &value) -> em::val {
                    return em::val(value.as<T>());
                };
            }
        }

        em::val cast(const core::Any &value) const {
            auto it = casts_.find(value.get_type_index());
            if (it != casts_.end()) {
                return it->second(value);
            }
            return em::val::undefined();
        }

        bool has_cast(std::type_index type) const { return casts_.find(type) != casts_.end(); }

    private:
        std::unordered_map<std::type_index, CastFunc> casts_;

    public:
        // Allow registering custom cast functions
        void register_custom_cast(std::type_index type, CastFunc func) {
            casts_[type] = std::move(func);
        }
    };

    // ============================================================================
    // Type Converter Registry - For converting em::val to Any (reverse direction)
    // ============================================================================

    class TypeConverterRegistry {
    public:
        using ConverterFunc = std::function<core::Any(const em::val &)>;

        static TypeConverterRegistry &instance() {
            static TypeConverterRegistry reg;
            return reg;
        }

        template <typename T> void register_converter() {
            // For bound classes, use Emscripten's .as<T>() to extract the C++ object
            if constexpr (!std::is_abstract_v<T>) {
                converters_[std::type_index(typeid(T))] = [](const em::val &js_val) -> core::Any {
                    try {
                        // Emscripten's .as<T>() will extract the C++ object
                        return core::Any(js_val.as<T>());
                    } catch (const std::exception &e) {
                        throw std::runtime_error(
                            std::string("Failed to convert JavaScript object to C++ type: ") +
                            e.what());
                    }
                };
            }
        }

        core::Any convert(const em::val &js_val, std::type_index type) const {
            auto it = converters_.find(type);
            if (it != converters_.end()) {
                return it->second(js_val);
            }
            return core::Any();
        }

        bool has_converter(std::type_index type) const {
            return converters_.find(type) != converters_.end();
        }

        void register_custom_converter(std::type_index type, ConverterFunc func) {
            converters_[type] = std::move(func);
        }

    private:
        std::unordered_map<std::type_index, ConverterFunc> converters_;
    };

    // ============================================================================
    // Vector Type Registration Helpers
    // ============================================================================

    /**
     * @brief Register a vector type converter for a custom class
     * @tparam T The element type (must be already bound to Emscripten)
     *
     * Usage:
     *   bind_vector_type<Point>();
     *   bind_vector_type<Triangle>();
     */
    template <typename T> inline void bind_vector_type() {
        TypeCastRegistry::instance().register_custom_cast(
            std::type_index(typeid(std::vector<T>)), [](const core::Any &value) -> em::val {
                const auto &vec = value.as<std::vector<T>>();
                em::val     arr = em::val::array();
                for (size_t i = 0; i < vec.size(); ++i) {
                    arr.set(i, em::val(vec[i]));
                }
                return arr;
            });
    }

    // ============================================================================
    // Function Callback Registration Helpers
    // ============================================================================

    /**
     * @brief Register a std::function type converter for callbacks
     * @tparam Ret Return type
     * @tparam Args Argument types
     *
     * This allows JavaScript functions to be passed as C++ std::function parameters.
     *
     * Usage:
     *   bind_function_type<Point, Point>();  // For std::function<Point(Point)>
     *   bind_function_type<void, int, double>();  // For std::function<void(int, double)>
     */
    template <typename Ret, typename... Args> inline void bind_function_type() {
        using FuncType = std::function<Ret(Args...)>;

        // Register converter: JavaScript function -> C++ std::function
        TypeConverterRegistry::instance().register_custom_converter(
            std::type_index(typeid(FuncType)), [](const em::val &js_func) -> core::Any {
                // Wrap the JavaScript function in a C++ std::function
                FuncType cpp_func = [js_func](Args... args) -> Ret {
                    try {
                        // Call the JavaScript function with converted arguments
                        em::val result = js_func(em::val(args)...);

                        if constexpr (std::is_void_v<Ret>) {
                            return;
                        } else {
                            // Convert the result back to C++
                            return result.as<Ret>();
                        }
                    } catch (const std::exception &e) {
                        throw std::runtime_error(std::string("JavaScript callback failed: ") +
                                                 e.what());
                    }
                };

                return core::Any(cpp_func);
            });
    }

    // ============================================================================
    // Inheritance Registry - Track class inheritance relationships
    // ============================================================================

    class InheritanceRegistry {
    public:
        static InheritanceRegistry &instance() {
            static InheritanceRegistry reg;
            return reg;
        }

        // Register a class with its base classes
        void register_class(const std::string &class_name, std::type_index class_type,
                            const std::vector<std::string> &base_names = {}) {
            class_types_.insert_or_assign(class_name, class_type);
            type_to_name_.insert_or_assign(class_type, class_name);
            base_classes_[class_name] = base_names;

            // Register derived relationship
            for (const auto &base : base_names) {
                derived_classes_[base].push_back(class_name);
            }
        }

        // Get base class names for a class
        std::vector<std::string> get_base_classes(const std::string &class_name) const {
            auto it = base_classes_.find(class_name);
            if (it != base_classes_.end()) {
                return it->second;
            }
            return {};
        }

        // Get primary base class (first one)
        std::string get_primary_base(const std::string &class_name) const {
            auto bases = get_base_classes(class_name);
            return bases.empty() ? "" : bases[0];
        }

        // Get derived class names for a class
        std::vector<std::string> get_derived_classes(const std::string &class_name) const {
            auto it = derived_classes_.find(class_name);
            if (it != derived_classes_.end()) {
                return it->second;
            }
            return {};
        }

        // Check if class inherits from another (direct or indirect)
        bool inherits_from(const std::string &derived, const std::string &base) const {
            if (derived == base)
                return true;

            auto bases = get_base_classes(derived);
            for (const auto &b : bases) {
                if (b == base || inherits_from(b, base)) {
                    return true;
                }
            }
            return false;
        }

        // Get all ancestor classes
        std::vector<std::string> get_all_ancestors(const std::string &class_name) const {
            std::vector<std::string>        ancestors;
            std::unordered_set<std::string> visited;
            collect_ancestors(class_name, ancestors, visited);
            return ancestors;
        }

        // Get class name from type
        std::string get_class_name(std::type_index type) const {
            auto it = type_to_name_.find(type);
            if (it != type_to_name_.end()) {
                return it->second;
            }
            return "";
        }

        // Get type from class name
        std::type_index get_class_type(const std::string &name) const {
            auto it = class_types_.find(name);
            if (it != class_types_.end()) {
                return it->second;
            }
            return std::type_index(typeid(void));
        }

        // Check if a class is registered
        bool is_registered(const std::string &name) const {
            return class_types_.find(name) != class_types_.end();
        }

    private:
        void collect_ancestors(const std::string &class_name, std::vector<std::string> &ancestors,
                               std::unordered_set<std::string> &visited) const {
            if (visited.count(class_name))
                return;
            visited.insert(class_name);

            auto bases = get_base_classes(class_name);
            for (const auto &base : bases) {
                ancestors.push_back(base);
                collect_ancestors(base, ancestors, visited);
            }
        }

        std::unordered_map<std::string, std::type_index>          class_types_;
        std::unordered_map<std::type_index, std::string>          type_to_name_;
        std::unordered_map<std::string, std::vector<std::string>> base_classes_;
        std::unordered_map<std::string, std::vector<std::string>> derived_classes_;
    };

    // ============================================================================
    // TypeScript Declaration Generator
    // ============================================================================

    class TypeScriptGenerator {
    public:
        static TypeScriptGenerator &instance() {
            static TypeScriptGenerator gen;
            return gen;
        }

        void add_class(const std::string &name, const std::vector<std::string> &fields,
                       const std::vector<std::string> &methods,
                       const std::string              &base_class = "") {
            ClassInfo info;
            info.name       = name;
            info.fields     = fields;
            info.methods    = methods;
            info.base_class = base_class;
            classes_[name]  = info;
        }

        // Add class with multiple base classes
        void add_class(const std::string &name, const std::vector<std::string> &fields,
                       const std::vector<std::string> &methods,
                       const std::vector<std::string> &base_classes) {
            ClassInfo info;
            info.name         = name;
            info.fields       = fields;
            info.methods      = methods;
            info.base_classes = base_classes;
            if (!base_classes.empty()) {
                info.base_class = base_classes[0];
            }
            classes_[name] = info;
        }

        void add_constructor(const std::string                  &class_name,
                             const std::vector<std::type_index> &arg_types) {
            constructor_types_[class_name].push_back(arg_types);
        }

        void add_field_type(const std::string &class_name, const std::string &field_name,
                            std::type_index type) {
            field_types_.insert_or_assign(class_name + "." + field_name, type);
        }

        void add_method_info(const std::string &class_name, const std::string &method_name,
                             std::type_index                     return_type,
                             const std::vector<std::type_index> &arg_types) {
            MethodInfo info;
            info.return_type                              = return_type;
            info.arg_types                                = arg_types;
            method_types_[class_name + "." + method_name] = info;
        }

        void add_function(const std::string &name, std::type_index return_type,
                          const std::vector<std::type_index> &arg_types) {
            MethodInfo info;
            info.return_type = return_type;
            info.arg_types   = arg_types;
            functions_[name] = info;
        }

        void set_type_name(std::type_index type, const std::string &name) {
            type_names_[type] = name;
        }

        void mark_abstract(const std::string &class_name) { abstract_classes_.insert(class_name); }

        void mark_polymorphic(const std::string &class_name) {
            polymorphic_classes_.insert(class_name);
        }

        std::string generate() const {
            std::ostringstream ts;

            ts << "// Auto-generated TypeScript declarations for Rosetta Emscripten bindings\n";
            ts << "// Generated by em_generator_v5 with automatic inheritance support\n\n";

            ts << "declare module 'Module' {\n";
            ts << "  export interface EmscriptenModule {\n";

            // Classes
            for (const auto &[name, info] : classes_) {
                ts << "    " << name << ": " << name << "Constructor;\n";
            }

            // Free functions
            for (const auto &[name, info] : functions_) {
                ts << "    " << name << "(";
                for (size_t i = 0; i < info.arg_types.size(); ++i) {
                    if (i > 0)
                        ts << ", ";
                    ts << "arg" << i << ": " << type_to_ts(info.arg_types[i]);
                }
                ts << "): " << type_to_ts(info.return_type) << ";\n";
            }

            // Utility functions
            ts << "    listClasses(): string[];\n";
            ts << "    version(): string;\n";
            ts << "    generateTypeScript(): string;\n";
            ts << "    getInheritanceInfo(className: string): { bases: string[], derived: string[] "
                  "};\n";

            ts << "  }\n\n";

            // Class interfaces
            for (const auto &[name, info] : classes_) {
                // Constructor interface
                ts << "  interface " << name << "Constructor {\n";

                if (abstract_classes_.find(name) == abstract_classes_.end()) {
                    auto ctor_it = constructor_types_.find(name);
                    if (ctor_it != constructor_types_.end() && !ctor_it->second.empty()) {
                        for (const auto &ctor_args : ctor_it->second) {
                            ts << "    new(";
                            for (size_t i = 0; i < ctor_args.size(); ++i) {
                                if (i > 0)
                                    ts << ", ";
                                ts << "arg" << i << ": " << type_to_ts(ctor_args[i]);
                            }
                            ts << "): " << name << ";\n";
                        }
                    } else {
                        ts << "    new(): " << name << ";\n";
                    }
                }

                ts << "    $meta(): { fields: string[], methods: string[], bases: string[] };\n";
                ts << "    $isAbstract(): boolean;\n";
                ts << "  }\n\n";

                // Instance interface
                ts << "  interface " << name;

                if (!info.base_classes.empty()) {
                    ts << " extends " << info.base_classes[0];
                    for (size_t i = 1; i < info.base_classes.size(); ++i) {
                        ts << ", " << info.base_classes[i];
                    }
                } else if (!info.base_class.empty()) {
                    ts << " extends " << info.base_class;
                }
                ts << " {\n";

                // Fields
                for (const auto &field : info.fields) {
                    std::string key     = name + "." + field;
                    auto        it      = field_types_.find(key);
                    std::string ts_type = "any";
                    if (it != field_types_.end()) {
                        ts_type = type_to_ts(it->second);
                    }
                    ts << "    " << field << ": " << ts_type << ";\n";
                }

                // Methods
                for (const auto &method : info.methods) {
                    std::string key = name + "." + method;
                    auto        it  = method_types_.find(key);
                    if (it != method_types_.end()) {
                        ts << "    " << method << "(";
                        for (size_t i = 0; i < it->second.arg_types.size(); ++i) {
                            if (i > 0)
                                ts << ", ";
                            ts << "arg" << i << ": " << type_to_ts(it->second.arg_types[i]);
                        }
                        ts << "): " << type_to_ts(it->second.return_type) << ";\n";
                    } else {
                        ts << "    " << method << "(...args: any[]): any;\n";
                    }
                }

                ts << "    delete(): void;\n";
                ts << "  }\n\n";
            }

            ts << "  export default function(): Promise<EmscriptenModule>;\n";
            ts << "}\n";

            return ts.str();
        }

    private:
        struct ClassInfo {
            std::string              name;
            std::vector<std::string> fields;
            std::vector<std::string> methods;
            std::string              base_class;
            std::vector<std::string> base_classes;
        };

        struct MethodInfo {
            std::type_index              return_type = std::type_index(typeid(void));
            std::vector<std::type_index> arg_types;
        };

        std::map<std::string, ClassInfo>                                 classes_;
        std::map<std::string, std::vector<std::vector<std::type_index>>> constructor_types_;
        std::map<std::string, std::type_index>                           field_types_;
        std::map<std::string, MethodInfo>                                method_types_;
        std::map<std::string, MethodInfo>                                functions_;
        std::map<std::type_index, std::string>                           type_names_;
        std::unordered_set<std::string>                                  abstract_classes_;
        std::unordered_set<std::string>                                  polymorphic_classes_;

        std::string type_to_ts(std::type_index type) const {
            auto custom_it = type_names_.find(type);
            if (custom_it != type_names_.end()) {
                return custom_it->second;
            }

            if (type == std::type_index(typeid(int)))
                return "number";
            if (type == std::type_index(typeid(double)))
                return "number";
            if (type == std::type_index(typeid(float)))
                return "number";
            if (type == std::type_index(typeid(size_t)))
                return "number";
            if (type == std::type_index(typeid(long)))
                return "number";
            if (type == std::type_index(typeid(long long)))
                return "number";
            if (type == std::type_index(typeid(unsigned int)))
                return "number";
            if (type == std::type_index(typeid(unsigned long)))
                return "number";
            if (type == std::type_index(typeid(short)))
                return "number";
            if (type == std::type_index(typeid(char)))
                return "number";
            if (type == std::type_index(typeid(bool)))
                return "boolean";
            if (type == std::type_index(typeid(std::string)))
                return "string";
            if (type == std::type_index(typeid(void)))
                return "void";

            if (type == std::type_index(typeid(std::vector<double>)))
                return "number[]";
            if (type == std::type_index(typeid(std::vector<float>)))
                return "number[]";
            if (type == std::type_index(typeid(std::vector<int>)))
                return "number[]";
            if (type == std::type_index(typeid(std::vector<size_t>)))
                return "number[]";
            if (type == std::type_index(typeid(std::vector<std::string>)))
                return "string[]";
            if (type == std::type_index(typeid(std::vector<bool>)))
                return "boolean[]";

            return "any";
        }
    };

    // ============================================================================
    // Type Converters
    // ============================================================================

    inline em::val any_to_val(const core::Any &value) {
        if (!value.has_value())
            return em::val::undefined();

        auto type = value.get_type_index();

        if (type == std::type_index(typeid(int)))
            return em::val(value.as<int>());
        if (type == std::type_index(typeid(double)))
            return em::val(value.as<double>());
        if (type == std::type_index(typeid(float)))
            return em::val(value.as<float>());
        if (type == std::type_index(typeid(bool)))
            return em::val(value.as<bool>());
        if (type == std::type_index(typeid(std::string)))
            return em::val(value.as<std::string>());
        if (type == std::type_index(typeid(size_t)))
            return em::val(static_cast<unsigned long>(value.as<size_t>()));
        if (type == std::type_index(typeid(long)))
            return em::val(value.as<long>());
        if (type == std::type_index(typeid(void)))
            return em::val::undefined();

        if (type == std::type_index(typeid(std::vector<double>))) {
            const auto &vec = value.as<std::vector<double>>();
            em::val     arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i)
                arr.set(i, vec[i]);
            return arr;
        }
        if (type == std::type_index(typeid(std::vector<int>))) {
            const auto &vec = value.as<std::vector<int>>();
            em::val     arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i)
                arr.set(i, vec[i]);
            return arr;
        }
        if (type == std::type_index(typeid(std::vector<std::string>))) {
            const auto &vec = value.as<std::vector<std::string>>();
            em::val     arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i)
                arr.set(i, vec[i]);
            return arr;
        }

        if (TypeCastRegistry::instance().has_cast(type)) {
            return TypeCastRegistry::instance().cast(value);
        }

        return em::val::undefined();
    }

    inline core::Any val_to_any(const em::val &js_val, std::type_index expected_type) {
        // Check for function types FIRST
        std::string type_of = js_val.typeOf().as<std::string>();
        if (type_of == "function") {
            // // DEBUG: Print the expected type
            // std::cerr << "Detected JS function, expected type: " << expected_type.name()
            //           << std::endl;
            // std::cerr << "Has converter: "
            //           << TypeConverterRegistry::instance().has_converter(expected_type)
            //           << std::endl;
            if (TypeConverterRegistry::instance().has_converter(expected_type)) {
                return TypeConverterRegistry::instance().convert(js_val, expected_type);
            }
        }

        if (expected_type == std::type_index(typeid(int)))
            return core::Any(js_val.as<int>());
        if (expected_type == std::type_index(typeid(double)))
            return core::Any(js_val.as<double>());
        if (expected_type == std::type_index(typeid(float)))
            return core::Any(js_val.as<float>());
        if (expected_type == std::type_index(typeid(bool)))
            return core::Any(js_val.as<bool>());
        if (expected_type == std::type_index(typeid(std::string)))
            return core::Any(js_val.as<std::string>());
        if (expected_type == std::type_index(typeid(size_t)))
            return core::Any(static_cast<size_t>(js_val.as<unsigned long>()));

        if (expected_type == std::type_index(typeid(std::vector<double>))) {
            std::vector<double> vec;
            unsigned            length = js_val["length"].as<unsigned>();
            for (unsigned i = 0; i < length; ++i)
                vec.push_back(js_val[i].as<double>());
            return core::Any(vec);
        }
        if (expected_type == std::type_index(typeid(std::vector<int>))) {
            std::vector<int> vec;
            unsigned         length = js_val["length"].as<unsigned>();
            for (unsigned i = 0; i < length; ++i)
                vec.push_back(js_val[i].as<int>());
            return core::Any(vec);
        }

        // Try custom type converter registry for bound classes
        if (TypeConverterRegistry::instance().has_converter(expected_type)) {
            return TypeConverterRegistry::instance().convert(js_val, expected_type);
        }

        throw std::runtime_error("Unsupported type for JavaScript conversion");
    }

    // ============================================================================
    // Constructor Binder
    // ============================================================================

    template <typename T, typename TupleType> struct ConstructorBinder;

    template <typename T, typename... Args> struct ConstructorBinder<T, std::tuple<Args...>> {
        template <typename ClassType> static void bind(ClassType &em_class) {
            em_class.template constructor<Args...>();
        }

        static void register_typescript(const std::string &class_name) {
            std::vector<std::type_index> arg_types = {std::type_index(typeid(Args))...};
            TypeScriptGenerator::instance().add_constructor(class_name, arg_types);
        }
    };

    // ============================================================================
    // Class Wrapper - Provides generic access via $get/$set/$call
    // ============================================================================

    template <typename T> struct ClassWrapper {
        static em::val getField(T &obj, const std::string &name) {
            const auto &meta = core::Registry::instance().get<T>();
            try {
                core::Any value = meta.get_field(obj, name);
                return any_to_val(value);
            } catch (const std::exception &e) {
                throw std::runtime_error("Field '" + name + "' not found: " + e.what());
            }
        }

        static void setField(T &obj, const std::string &name, em::val value) {
            const auto &meta = core::Registry::instance().get<T>();
            try {
                auto      field_type = meta.get_field_type(name);
                core::Any cpp_val    = val_to_any(value, field_type);
                meta.set_field(obj, name, cpp_val);
            } catch (const std::exception &e) {
                throw std::runtime_error("Cannot set field '" + name + "': " + e.what());
            }
        }

        // Method caller with inheritance support
        static em::val callMethod(T &obj, const std::string &name, em::val args) {
            const auto &meta = core::Registry::instance().get<T>();

            try {
                size_t                       arity = 0;
                std::vector<std::type_index> arg_types;
                bool                         method_info_found = false;

                // First, try to get method info from current class
                const auto &methods = meta.methods();
                bool        name_in_methods =
                    std::find(methods.begin(), methods.end(), name) != methods.end();

                if (name_in_methods) {
                    try {
                        arity             = meta.get_method_arity(name);
                        arg_types         = meta.get_method_arg_types(name);
                        method_info_found = true;
                    } catch (...) {
                    }
                }

                // If not found locally, search base classes
                if (!method_info_found) {
                    const auto &inheritance = meta.inheritance();

                    for (const auto &base_info : inheritance.base_classes) {
                        auto *base_holder = core::Registry::instance().get_by_name(base_info.name);
                        if (!base_holder)
                            continue;

                        if (base_holder->has_method(name)) {
                            try {
                                arity             = base_holder->get_method_arity(name);
                                arg_types         = base_holder->get_method_arg_types(name);
                                method_info_found = true;
                                break;
                            } catch (...) {
                            }
                        }
                    }

                    if (!method_info_found) {
                        for (const auto &base_info : inheritance.virtual_bases) {
                            auto *base_holder =
                                core::Registry::instance().get_by_name(base_info.name);
                            if (!base_holder)
                                continue;

                            if (base_holder->has_method(name)) {
                                try {
                                    arity             = base_holder->get_method_arity(name);
                                    arg_types         = base_holder->get_method_arg_types(name);
                                    method_info_found = true;
                                    break;
                                } catch (...) {
                                }
                            }
                        }
                    }
                }

                // Fallback: infer from JS args
                if (!method_info_found) {
                    unsigned js_length = args["length"].as<unsigned>();
                    arity              = js_length;

                    for (unsigned i = 0; i < js_length; ++i) {
                        em::val arg = args[i];
                        if (arg.isNumber()) {
                            arg_types.push_back(std::type_index(typeid(double)));
                        } else if (arg.isString()) {
                            arg_types.push_back(std::type_index(typeid(std::string)));
                        } else if (arg.isTrue() || arg.isFalse()) {
                            arg_types.push_back(std::type_index(typeid(bool)));
                        } else {
                            arg_types.push_back(std::type_index(typeid(double)));
                        }
                    }
                }

                std::vector<core::Any> cpp_args;
                for (size_t i = 0; i < arity; ++i) {
                    cpp_args.push_back(val_to_any(args[i], arg_types[i]));
                }

                core::Any result = meta.invoke_method(obj, name, cpp_args);
                return any_to_val(result);
            } catch (const std::exception &e) {
                throw std::runtime_error("Method '" + name + "' call failed: " + e.what());
            }
        }

        static em::val getMetadata() {
            const auto &meta = core::Registry::instance().get<T>();
            em::val     info = em::val::object();

            em::val fields = em::val::array();
            for (const auto &f : meta.fields()) {
                fields.call<void>("push", em::val(f));
            }
            info.set("fields", fields);

            em::val methods = em::val::array();
            for (const auto &m : meta.methods()) {
                methods.call<void>("push", em::val(m));
            }
            info.set("methods", methods);

            em::val     bases       = em::val::array();
            const auto &inheritance = meta.inheritance();
            for (const auto &base : inheritance.base_classes) {
                bases.call<void>("push", em::val(base.name));
            }
            info.set("bases", bases);

            info.set("isAbstract", inheritance.is_abstract);
            info.set("isPolymorphic", inheritance.is_polymorphic);
            info.set("hasVirtualDestructor", inheritance.has_virtual_destructor);

            return info;
        }

        static bool isAbstract() {
            const auto &meta = core::Registry::instance().get<T>();
            return meta.inheritance().is_abstract;
        }

        static bool instanceOf(T &obj, const std::string &class_name) {
            std::string my_name = core::Registry::instance().get<T>().name();
            return InheritanceRegistry::instance().inherits_from(my_name, class_name);
        }
    };

    // ============================================================================
    // Static storage for method names (needed for lambda captures)
    // ============================================================================

    inline std::vector<std::string> &getMethodNameStorage() {
        static std::vector<std::string> storage;
        return storage;
    }

    // ============================================================================
    // Static storage for field names (needed for lambda captures)
    // ============================================================================

    inline std::vector<std::string> &getFieldNameStorage() {
        static std::vector<std::string> storage;
        return storage;
    }

    // ============================================================================
    // Helper to collect methods from inheritance chain
    // ============================================================================

    inline void collectMethodsFromBases(const core::InheritanceInfo &inheritance,
                                        std::set<std::string>       &all_methods) {
        for (const auto &base_info : inheritance.base_classes) {
            auto *base_holder = core::Registry::instance().get_by_name(base_info.name);
            if (!base_holder)
                continue;

            for (const auto &method : base_holder->get_methods()) {
                all_methods.insert(method);
            }

            collectMethodsFromBases(base_holder->get_inheritance(), all_methods);
        }

        for (const auto &base_info : inheritance.virtual_bases) {
            auto *base_holder = core::Registry::instance().get_by_name(base_info.name);
            if (!base_holder)
                continue;

            for (const auto &method : base_holder->get_methods()) {
                all_methods.insert(method);
            }

            collectMethodsFromBases(base_holder->get_inheritance(), all_methods);
        }
    }

    // ============================================================================
    // Auto Class Binder with Base Class Support
    // ============================================================================

    template <typename T, typename Base, typename... CtorSignatures> class AutoBinderWithBase {
    public:
        static void bind(const std::string &js_name) {
            const auto &meta = core::Registry::instance().get<T>();
            std::string name = js_name.empty() ? meta.name() : js_name;

            TypeCastRegistry::instance().register_cast<T>();
            TypeConverterRegistry::instance().register_converter<T>();
            TypeScriptGenerator::instance().set_type_name(std::type_index(typeid(T)), name);

            std::string base_name = core::Registry::instance().get<Base>().name();
            InheritanceRegistry::instance().register_class(name, std::type_index(typeid(T)),
                                                           {base_name});

            auto em_class = em::class_<T, em::base<Base>>(name.c_str());

            // Bind constructors
            if constexpr (!std::is_abstract_v<T>) {
                if constexpr (sizeof...(CtorSignatures) > 0) {
                    (ConstructorBinder<T, CtorSignatures>::bind(em_class), ...);
                    (ConstructorBinder<T, CtorSignatures>::register_typescript(name), ...);
                } else {
                    const auto &ctors = meta.constructor_infos();
                    for (const auto &ctor : ctors) {
                        if (ctor.arity == 0) {
                            em_class.template constructor<>();
                            TypeScriptGenerator::instance().add_constructor(name, {});
                            break;
                        }
                    }
                }
            } else {
                TypeScriptGenerator::instance().mark_abstract(name);
            }

            // Bind generic accessors
            em_class.function("$get", &ClassWrapper<T>::getField);
            em_class.function("$set", &ClassWrapper<T>::setField);
            em_class.function("$call", &ClassWrapper<T>::callMethod);
            em_class.class_function("$meta", &ClassWrapper<T>::getMetadata);
            em_class.class_function("$isAbstract", &ClassWrapper<T>::isAbstract);
            em_class.function("$instanceof", &ClassWrapper<T>::instanceOf);

            // *** AUTO-BIND ALL FIELDS AS PROPERTIES ***
            bind_all_fields(em_class, meta);

            // *** AUTO-BIND ALL METHODS INCLUDING INHERITED ***
            bind_all_methods(em_class, meta);

            // TypeScript info
            TypeScriptGenerator::instance().add_class(name, meta.fields(), meta.methods(),
                                                      base_name);

            if (meta.inheritance().is_polymorphic) {
                TypeScriptGenerator::instance().mark_polymorphic(name);
            }

            for (const auto &field : meta.fields()) {
                auto field_type = meta.get_field_type(field);
                TypeScriptGenerator::instance().add_field_type(name, field, field_type);
            }

            for (const auto &method : meta.methods()) {
                try {
                    auto return_type = meta.get_method_return_type(method);
                    auto arg_types   = meta.get_method_arg_types(method);
                    TypeScriptGenerator::instance().add_method_info(name, method, return_type,
                                                                    arg_types);
                } catch (...) {
                }
            }
        }

    private:
        template <typename EmClass>
        static void bind_all_fields(EmClass &em_class, const core::ClassMetadata<T> &meta) {
            auto  &storage     = getFieldNameStorage();
            size_t start_index = storage.size();

            // Store field names
            for (const auto &field_name : meta.fields()) {
                storage.push_back(field_name);
            }

            // Bind each field as a property
            size_t idx = start_index;
            for (const auto &field_name : meta.fields()) {
                size_t field_idx = idx++;

                // Create getter function
                auto getter = std::function<em::val(const T &)>([field_idx](
                                                                    const T &obj) -> em::val {
                    const std::string &fname = getFieldNameStorage()[field_idx];
                    const auto        &fmeta = core::Registry::instance().get<T>();
                    try {
                        T        &mutable_obj = const_cast<T &>(obj);
                        core::Any value       = fmeta.get_field(mutable_obj, fname);
                        return any_to_val(value);
                    } catch (const std::exception &e) {
                        throw std::runtime_error("Field '" + fname + "' get failed: " + e.what());
                    }
                });

                // Create setter function
                auto setter = std::function<void(T &, em::val)>([field_idx](T &obj, em::val value) {
                    const std::string &fname = getFieldNameStorage()[field_idx];
                    const auto        &fmeta = core::Registry::instance().get<T>();
                    try {
                        auto      field_type = fmeta.get_field_type(fname);
                        core::Any cpp_val    = val_to_any(value, field_type);
                        fmeta.set_field(obj, fname, cpp_val);
                    } catch (const std::exception &e) {
                        throw std::runtime_error("Field '" + fname + "' set failed: " + e.what());
                    }
                });

                // Bind the property
                em_class.property(field_name.c_str(), getter, setter);
            }
        }

        template <typename EmClass>
        static void bind_all_methods(EmClass &em_class, const core::ClassMetadata<T> &meta) {
            std::set<std::string> all_methods;

            for (const auto &method : meta.methods()) {
                all_methods.insert(method);
            }

            collectMethodsFromBases(meta.inheritance(), all_methods);

            auto  &storage     = getMethodNameStorage();
            size_t start_index = storage.size();

            for (const auto &method_name : all_methods) {
                storage.push_back(method_name);
            }

            size_t idx = start_index;
            for (const auto &method_name : all_methods) {
                size_t method_idx = idx++;

                // Determine the arity of this method
                size_t arity       = 0;
                bool   found_arity = false;

                // First check if it's in the current class
                const auto &methods = meta.methods();
                if (std::find(methods.begin(), methods.end(), method_name) != methods.end()) {
                    try {
                        arity       = meta.get_method_arity(method_name);
                        found_arity = true;
                    } catch (...) {
                    }
                }

                // If not found, check base classes
                if (!found_arity) {
                    const auto &inheritance = meta.inheritance();

                    for (const auto &base_info : inheritance.base_classes) {
                        auto *base_holder = core::Registry::instance().get_by_name(base_info.name);
                        if (base_holder && base_holder->has_method(method_name)) {
                            try {
                                arity       = base_holder->get_method_arity(method_name);
                                found_arity = true;
                                break;
                            } catch (...) {
                            }
                        }
                    }

                    if (!found_arity) {
                        for (const auto &base_info : inheritance.virtual_bases) {
                            auto *base_holder =
                                core::Registry::instance().get_by_name(base_info.name);
                            if (base_holder && base_holder->has_method(method_name)) {
                                try {
                                    arity       = base_holder->get_method_arity(method_name);
                                    found_arity = true;
                                    break;
                                } catch (...) {
                                }
                            }
                        }
                    }
                }

                // Bind method based on arity
                switch (arity) {
                case 0:
                    em_class.function(
                        method_name.c_str(),
                        std::function<em::val(T &)>([method_idx](T &obj) -> em::val {
                            const std::string &mname = getMethodNameStorage()[method_idx];
                            return ClassWrapper<T>::callMethod(obj, mname, em::val::array());
                        }));
                    break;

                case 1:
                    em_class.function(method_name.c_str(),
                                      std::function<em::val(T &, em::val)>(
                                          [method_idx](T &obj, em::val arg0) -> em::val {
                                              const std::string &mname =
                                                  getMethodNameStorage()[method_idx];
                                              em::val args = em::val::array();
                                              args.call<void>("push", arg0);
                                              return ClassWrapper<T>::callMethod(obj, mname, args);
                                          }));
                    break;

                case 2:
                    em_class.function(
                        method_name.c_str(),
                        std::function<em::val(T &, em::val, em::val)>(
                            [method_idx](T &obj, em::val arg0, em::val arg1) -> em::val {
                                const std::string &mname = getMethodNameStorage()[method_idx];
                                em::val            args  = em::val::array();
                                args.call<void>("push", arg0);
                                args.call<void>("push", arg1);
                                return ClassWrapper<T>::callMethod(obj, mname, args);
                            }));
                    break;

                case 3:
                    em_class.function(method_name.c_str(),
                                      std::function<em::val(T &, em::val, em::val, em::val)>(
                                          [method_idx](T &obj, em::val arg0, em::val arg1,
                                                       em::val arg2) -> em::val {
                                              const std::string &mname =
                                                  getMethodNameStorage()[method_idx];
                                              em::val args = em::val::array();
                                              args.call<void>("push", arg0);
                                              args.call<void>("push", arg1);
                                              args.call<void>("push", arg2);
                                              return ClassWrapper<T>::callMethod(obj, mname, args);
                                          }));
                    break;

                case 4:
                    em_class.function(
                        method_name.c_str(),
                        std::function<em::val(T &, em::val, em::val, em::val, em::val)>(
                            [method_idx](T &obj, em::val arg0, em::val arg1, em::val arg2,
                                         em::val arg3) -> em::val {
                                const std::string &mname = getMethodNameStorage()[method_idx];
                                em::val            args  = em::val::array();
                                args.call<void>("push", arg0);
                                args.call<void>("push", arg1);
                                args.call<void>("push", arg2);
                                args.call<void>("push", arg3);
                                return ClassWrapper<T>::callMethod(obj, mname, args);
                            }));
                    break;

                case 5:
                    em_class.function(
                        method_name.c_str(),
                        std::function<em::val(T &, em::val, em::val, em::val, em::val, em::val)>(
                            [method_idx](T &obj, em::val a0, em::val a1, em::val a2, em::val a3,
                                         em::val a4) -> em::val {
                                const std::string &mname = getMethodNameStorage()[method_idx];
                                em::val            args  = em::val::array();
                                args.call<void>("push", a0);
                                args.call<void>("push", a1);
                                args.call<void>("push", a2);
                                args.call<void>("push", a3);
                                args.call<void>("push", a4);
                                return ClassWrapper<T>::callMethod(obj, mname, args);
                            }));
                    break;

                default:
                    // For arity > 5, fall back to taking an array argument
                    em_class.function(method_name.c_str(),
                                      std::function<em::val(T &, em::val)>(
                                          [method_idx](T &obj, em::val args) -> em::val {
                                              const std::string &mname =
                                                  getMethodNameStorage()[method_idx];
                                              return ClassWrapper<T>::callMethod(obj, mname, args);
                                          }));
                    break;
                }
            }
        }
    };

    // ============================================================================
    // Auto Class Binder (no explicit base)
    // ============================================================================

    template <typename T, typename... CtorSignatures> class AutoBinder {
    public:
        static void bind(const std::string &js_name) {
            const auto &meta = core::Registry::instance().get<T>();
            std::string name = js_name.empty() ? meta.name() : js_name;

            TypeCastRegistry::instance().register_cast<T>();
            TypeConverterRegistry::instance().register_converter<T>();
            TypeScriptGenerator::instance().set_type_name(std::type_index(typeid(T)), name);
            InheritanceRegistry::instance().register_class(name, std::type_index(typeid(T)));

            auto em_class = em::class_<T>(name.c_str());

            // Bind constructors
            if constexpr (!std::is_abstract_v<T>) {
                if constexpr (sizeof...(CtorSignatures) > 0) {
                    (ConstructorBinder<T, CtorSignatures>::bind(em_class), ...);
                    (ConstructorBinder<T, CtorSignatures>::register_typescript(name), ...);
                } else {
                    const auto &ctors = meta.constructor_infos();
                    for (const auto &ctor : ctors) {
                        if (ctor.arity == 0) {
                            em_class.template constructor<>();
                            TypeScriptGenerator::instance().add_constructor(name, {});
                            break;
                        }
                    }
                }
            } else {
                TypeScriptGenerator::instance().mark_abstract(name);
            }

            // Bind generic accessors
            em_class.function("$get", &ClassWrapper<T>::getField);
            em_class.function("$set", &ClassWrapper<T>::setField);
            em_class.function("$call", &ClassWrapper<T>::callMethod);
            em_class.class_function("$meta", &ClassWrapper<T>::getMetadata);
            em_class.class_function("$isAbstract", &ClassWrapper<T>::isAbstract);
            em_class.function("$instanceof", &ClassWrapper<T>::instanceOf);

            // *** AUTO-BIND ALL FIELDS AS PROPERTIES ***
            bind_all_fields(em_class, meta);

            // *** AUTO-BIND ALL METHODS INCLUDING INHERITED ***
            bind_all_methods(em_class, meta);

            // TypeScript info
            const auto              &inheritance = meta.inheritance();
            std::vector<std::string> base_names;
            for (const auto &base : inheritance.base_classes) {
                base_names.push_back(base.name);
            }

            if (base_names.empty()) {
                TypeScriptGenerator::instance().add_class(name, meta.fields(), meta.methods());
            } else {
                TypeScriptGenerator::instance().add_class(name, meta.fields(), meta.methods(),
                                                          base_names);
            }

            if (inheritance.is_polymorphic) {
                TypeScriptGenerator::instance().mark_polymorphic(name);
            }
            if (inheritance.is_abstract) {
                TypeScriptGenerator::instance().mark_abstract(name);
            }

            for (const auto &field : meta.fields()) {
                auto field_type = meta.get_field_type(field);
                TypeScriptGenerator::instance().add_field_type(name, field, field_type);
            }

            for (const auto &method : meta.methods()) {
                try {
                    auto return_type = meta.get_method_return_type(method);
                    auto arg_types   = meta.get_method_arg_types(method);
                    TypeScriptGenerator::instance().add_method_info(name, method, return_type,
                                                                    arg_types);
                } catch (...) {
                }
            }
        }

    private:
        template <typename EmClass>
        static void bind_all_fields(EmClass &em_class, const core::ClassMetadata<T> &meta) {
            auto  &storage     = getFieldNameStorage();
            size_t start_index = storage.size();

            // Store field names
            for (const auto &field_name : meta.fields()) {
                storage.push_back(field_name);
            }

            // Bind each field as a property
            size_t idx = start_index;
            for (const auto &field_name : meta.fields()) {
                size_t field_idx = idx++;

                // Create getter function
                auto getter = std::function<em::val(const T &)>([field_idx](
                                                                    const T &obj) -> em::val {
                    const std::string &fname = getFieldNameStorage()[field_idx];
                    const auto        &fmeta = core::Registry::instance().get<T>();
                    try {
                        T        &mutable_obj = const_cast<T &>(obj);
                        core::Any value       = fmeta.get_field(mutable_obj, fname);
                        return any_to_val(value);
                    } catch (const std::exception &e) {
                        throw std::runtime_error("Field '" + fname + "' get failed: " + e.what());
                    }
                });

                // Create setter function
                auto setter = std::function<void(T &, em::val)>([field_idx](T &obj, em::val value) {
                    const std::string &fname = getFieldNameStorage()[field_idx];
                    const auto        &fmeta = core::Registry::instance().get<T>();
                    try {
                        auto      field_type = fmeta.get_field_type(fname);
                        core::Any cpp_val    = val_to_any(value, field_type);
                        fmeta.set_field(obj, fname, cpp_val);
                    } catch (const std::exception &e) {
                        throw std::runtime_error("Field '" + fname + "' set failed: " + e.what());
                    }
                });

                // Bind the property
                em_class.property(field_name.c_str(), getter, setter);
            }
        }
        template <typename EmClass>
        static void bind_all_methods(EmClass &em_class, const core::ClassMetadata<T> &meta) {
            std::set<std::string> all_methods;

            for (const auto &method : meta.methods()) {
                all_methods.insert(method);
            }

            collectMethodsFromBases(meta.inheritance(), all_methods);

            auto  &storage     = getMethodNameStorage();
            size_t start_index = storage.size();

            for (const auto &method_name : all_methods) {
                storage.push_back(method_name);
            }

            size_t idx = start_index;
            for (const auto &method_name : all_methods) {
                size_t method_idx = idx++;

                // Determine the arity of this method
                size_t arity       = 0;
                bool   found_arity = false;

                // First check if it's in the current class
                const auto &methods = meta.methods();
                if (std::find(methods.begin(), methods.end(), method_name) != methods.end()) {
                    try {
                        arity       = meta.get_method_arity(method_name);
                        found_arity = true;
                    } catch (...) {
                    }
                }

                // If not found, check base classes
                if (!found_arity) {
                    const auto &inheritance = meta.inheritance();

                    for (const auto &base_info : inheritance.base_classes) {
                        auto *base_holder = core::Registry::instance().get_by_name(base_info.name);
                        if (base_holder && base_holder->has_method(method_name)) {
                            try {
                                arity       = base_holder->get_method_arity(method_name);
                                found_arity = true;
                                break;
                            } catch (...) {
                            }
                        }
                    }

                    if (!found_arity) {
                        for (const auto &base_info : inheritance.virtual_bases) {
                            auto *base_holder =
                                core::Registry::instance().get_by_name(base_info.name);
                            if (base_holder && base_holder->has_method(method_name)) {
                                try {
                                    arity       = base_holder->get_method_arity(method_name);
                                    found_arity = true;
                                    break;
                                } catch (...) {
                                }
                            }
                        }
                    }
                }

                // Bind method based on arity
                switch (arity) {
                case 0:
                    em_class.function(
                        method_name.c_str(),
                        std::function<em::val(T &)>([method_idx](T &obj) -> em::val {
                            const std::string &mname = getMethodNameStorage()[method_idx];
                            return ClassWrapper<T>::callMethod(obj, mname, em::val::array());
                        }));
                    break;

                case 1:
                    em_class.function(method_name.c_str(),
                                      std::function<em::val(T &, em::val)>(
                                          [method_idx](T &obj, em::val arg0) -> em::val {
                                              const std::string &mname =
                                                  getMethodNameStorage()[method_idx];
                                              em::val args = em::val::array();
                                              args.call<void>("push", arg0);
                                              return ClassWrapper<T>::callMethod(obj, mname, args);
                                          }));
                    break;

                case 2:
                    em_class.function(
                        method_name.c_str(),
                        std::function<em::val(T &, em::val, em::val)>(
                            [method_idx](T &obj, em::val arg0, em::val arg1) -> em::val {
                                const std::string &mname = getMethodNameStorage()[method_idx];
                                em::val            args  = em::val::array();
                                args.call<void>("push", arg0);
                                args.call<void>("push", arg1);
                                return ClassWrapper<T>::callMethod(obj, mname, args);
                            }));
                    break;

                case 3:
                    em_class.function(method_name.c_str(),
                                      std::function<em::val(T &, em::val, em::val, em::val)>(
                                          [method_idx](T &obj, em::val arg0, em::val arg1,
                                                       em::val arg2) -> em::val {
                                              const std::string &mname =
                                                  getMethodNameStorage()[method_idx];
                                              em::val args = em::val::array();
                                              args.call<void>("push", arg0);
                                              args.call<void>("push", arg1);
                                              args.call<void>("push", arg2);
                                              return ClassWrapper<T>::callMethod(obj, mname, args);
                                          }));
                    break;

                case 4:
                    em_class.function(
                        method_name.c_str(),
                        std::function<em::val(T &, em::val, em::val, em::val, em::val)>(
                            [method_idx](T &obj, em::val arg0, em::val arg1, em::val arg2,
                                         em::val arg3) -> em::val {
                                const std::string &mname = getMethodNameStorage()[method_idx];
                                em::val            args  = em::val::array();
                                args.call<void>("push", arg0);
                                args.call<void>("push", arg1);
                                args.call<void>("push", arg2);
                                args.call<void>("push", arg3);
                                return ClassWrapper<T>::callMethod(obj, mname, args);
                            }));
                    break;

                case 5:
                    em_class.function(
                        method_name.c_str(),
                        std::function<em::val(T &, em::val, em::val, em::val, em::val, em::val)>(
                            [method_idx](T &obj, em::val a0, em::val a1, em::val a2, em::val a3,
                                         em::val a4) -> em::val {
                                const std::string &mname = getMethodNameStorage()[method_idx];
                                em::val            args  = em::val::array();
                                args.call<void>("push", a0);
                                args.call<void>("push", a1);
                                args.call<void>("push", a2);
                                args.call<void>("push", a3);
                                args.call<void>("push", a4);
                                return ClassWrapper<T>::callMethod(obj, mname, args);
                            }));
                    break;

                default:
                    // For arity > 5, fall back to taking an array argument
                    em_class.function(method_name.c_str(),
                                      std::function<em::val(T &, em::val)>(
                                          [method_idx](T &obj, em::val args) -> em::val {
                                              const std::string &mname =
                                                  getMethodNameStorage()[method_idx];
                                              return ClassWrapper<T>::callMethod(obj, mname, args);
                                          }));
                    break;
                }
            }
        }
    };

    // ============================================================================
    // Factory Constructor Support
    // ============================================================================

    // Helper to convert em::val arrays to std::vector
    struct EmValHelper {
        template <typename T> static std::vector<T> toVector(const em::val &arr) {
            std::vector<T> result;
            unsigned       length = arr["length"].as<unsigned>();
            result.reserve(length);
            for (unsigned i = 0; i < length; ++i) {
                result.push_back(arr[i].as<T>());
            }
            return result;
        }

        template <typename T> static em::val fromVector(const std::vector<T> &vec) {
            em::val arr = em::val::array();
            for (size_t i = 0; i < vec.size(); ++i) {
                arr.set(i, vec[i]);
            }
            return arr;
        }
    };

    // ============================================================================
    // Auto Binder with Factory Constructor Support
    // ============================================================================

    template <typename T, typename FactoryFunc, typename... CtorSignatures>
    class AutoBinderWithFactory {
    public:
        static void bind(const std::string &js_name, FactoryFunc factory) {
            const auto &meta = core::Registry::instance().get<T>();
            std::string name = js_name.empty() ? meta.name() : js_name;

            TypeCastRegistry::instance().register_cast<T>();
            TypeConverterRegistry::instance().register_converter<T>();
            TypeScriptGenerator::instance().set_type_name(std::type_index(typeid(T)), name);
            InheritanceRegistry::instance().register_class(name, std::type_index(typeid(T)));

            auto em_class = em::class_<T>(name.c_str());

            // Use factory function as constructor
            em_class.constructor(factory, em::allow_raw_pointers());

            // Bind generic accessors
            em_class.function("$get", &ClassWrapper<T>::getField);
            em_class.function("$set", &ClassWrapper<T>::setField);
            em_class.function("$call", &ClassWrapper<T>::callMethod);
            em_class.class_function("$meta", &ClassWrapper<T>::getMetadata);
            em_class.class_function("$isAbstract", &ClassWrapper<T>::isAbstract);
            em_class.function("$instanceof", &ClassWrapper<T>::instanceOf);

            // *** AUTO-BIND ALL FIELDS AS PROPERTIES ***
            bind_all_fields(em_class, meta);

            // Bind all methods
            bind_all_methods(em_class, meta);

            // TypeScript info
            TypeScriptGenerator::instance().add_class(name, meta.fields(), meta.methods());

            for (const auto &field : meta.fields()) {
                auto field_type = meta.get_field_type(field);
                TypeScriptGenerator::instance().add_field_type(name, field, field_type);
            }

            for (const auto &method : meta.methods()) {
                try {
                    auto return_type = meta.get_method_return_type(method);
                    auto arg_types   = meta.get_method_arg_types(method);
                    TypeScriptGenerator::instance().add_method_info(name, method, return_type,
                                                                    arg_types);
                } catch (...) {
                }
            }
        }

    private:
        template <typename EmClass>
        static void bind_all_fields(EmClass &em_class, const core::ClassMetadata<T> &meta) {
            auto  &storage     = getFieldNameStorage();
            size_t start_index = storage.size();

            // Store field names
            for (const auto &field_name : meta.fields()) {
                storage.push_back(field_name);
            }

            // Bind each field as a property
            size_t idx = start_index;
            for (const auto &field_name : meta.fields()) {
                size_t field_idx = idx++;

                // Create getter function
                auto getter = std::function<em::val(const T &)>([field_idx](
                                                                    const T &obj) -> em::val {
                    const std::string &fname = getFieldNameStorage()[field_idx];
                    const auto        &fmeta = core::Registry::instance().get<T>();
                    try {
                        T        &mutable_obj = const_cast<T &>(obj);
                        core::Any value       = fmeta.get_field(mutable_obj, fname);
                        return any_to_val(value);
                    } catch (const std::exception &e) {
                        throw std::runtime_error("Field '" + fname + "' get failed: " + e.what());
                    }
                });

                // Create setter function
                auto setter = std::function<void(T &, em::val)>([field_idx](T &obj, em::val value) {
                    const std::string &fname = getFieldNameStorage()[field_idx];
                    const auto        &fmeta = core::Registry::instance().get<T>();
                    try {
                        auto      field_type = fmeta.get_field_type(fname);
                        core::Any cpp_val    = val_to_any(value, field_type);
                        fmeta.set_field(obj, fname, cpp_val);
                    } catch (const std::exception &e) {
                        throw std::runtime_error("Field '" + fname + "' set failed: " + e.what());
                    }
                });

                // Bind the property
                em_class.property(field_name.c_str(), getter, setter);
            }
        }

        template <typename EmClass>
        static void bind_all_methods(EmClass &em_class, const core::ClassMetadata<T> &meta) {
            // Same implementation as AutoBinder::bind_all_methods
            std::set<std::string> all_methods;

            for (const auto &method : meta.methods()) {
                all_methods.insert(method);
            }

            collectMethodsFromBases(meta.inheritance(), all_methods);

            auto  &storage     = getMethodNameStorage();
            size_t start_index = storage.size();

            for (const auto &method_name : all_methods) {
                storage.push_back(method_name);
            }

            size_t idx = start_index;
            for (const auto &method_name : all_methods) {
                size_t method_idx = idx++;

                size_t arity       = 0;
                bool   found_arity = false;

                const auto &methods = meta.methods();
                if (std::find(methods.begin(), methods.end(), method_name) != methods.end()) {
                    try {
                        arity       = meta.get_method_arity(method_name);
                        found_arity = true;
                    } catch (...) {
                    }
                }

                if (!found_arity) {
                    const auto &inheritance = meta.inheritance();
                    for (const auto &base_info : inheritance.base_classes) {
                        auto *base_holder = core::Registry::instance().get_by_name(base_info.name);
                        if (base_holder && base_holder->has_method(method_name)) {
                            try {
                                arity       = base_holder->get_method_arity(method_name);
                                found_arity = true;
                                break;
                            } catch (...) {
                            }
                        }
                    }
                }

                // Bind based on arity (same switch as in AutoBinder)
                switch (arity) {
                case 0:
                    em_class.function(
                        method_name.c_str(),
                        std::function<em::val(T &)>([method_idx](T &obj) -> em::val {
                            const std::string &mname = getMethodNameStorage()[method_idx];
                            return ClassWrapper<T>::callMethod(obj, mname, em::val::array());
                        }));
                    break;
                case 1:
                    em_class.function(method_name.c_str(),
                                      std::function<em::val(T &, em::val)>(
                                          [method_idx](T &obj, em::val arg0) -> em::val {
                                              const std::string &mname =
                                                  getMethodNameStorage()[method_idx];
                                              em::val args = em::val::array();
                                              args.call<void>("push", arg0);
                                              return ClassWrapper<T>::callMethod(obj, mname, args);
                                          }));
                    break;
                // ... cases 2-5 same as AutoBinder
                default:
                    em_class.function(method_name.c_str(),
                                      std::function<em::val(T &, em::val)>(
                                          [method_idx](T &obj, em::val args) -> em::val {
                                              const std::string &mname =
                                                  getMethodNameStorage()[method_idx];
                                              return ClassWrapper<T>::callMethod(obj, mname, args);
                                          }));
                    break;
                }
            }
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

        template <typename T, typename FactoryFunc>
        EmGenerator &bind_class_with_factory(FactoryFunc factory, const std::string &js_name = "") {
            AutoBinderWithFactory<T, FactoryFunc>::bind(js_name, factory);
            return *this;
        }

        template <typename Derived, typename Base, typename... CtorSignatures>
        EmGenerator &bind_derived_class(const std::string &js_name = "") {
            AutoBinderWithBase<Derived, Base, CtorSignatures...>::bind(js_name);
            return *this;
        }

        template <typename T> EmGenerator &bind_abstract_class(const std::string &js_name = "") {
            const auto &meta = core::Registry::instance().get<T>();
            std::string name = js_name.empty() ? meta.name() : js_name;

            TypeScriptGenerator::instance().set_type_name(std::type_index(typeid(T)), name);
            InheritanceRegistry::instance().register_class(name, std::type_index(typeid(T)));

            auto em_class = em::class_<T>(name.c_str());

            em_class.function("$get", &ClassWrapper<T>::getField);
            em_class.function("$set", &ClassWrapper<T>::setField);
            em_class.function("$call", &ClassWrapper<T>::callMethod);
            em_class.class_function("$meta", &ClassWrapper<T>::getMetadata);
            em_class.class_function("$isAbstract", &ClassWrapper<T>::isAbstract);
            em_class.function("$instanceof", &ClassWrapper<T>::instanceOf);

            TypeScriptGenerator::instance().mark_abstract(name);
            TypeScriptGenerator::instance().add_class(name, meta.fields(), meta.methods());

            for (const auto &field : meta.fields()) {
                auto field_type = meta.get_field_type(field);
                TypeScriptGenerator::instance().add_field_type(name, field, field_type);
            }

            for (const auto &method : meta.methods()) {
                try {
                    auto return_type = meta.get_method_return_type(method);
                    auto arg_types   = meta.get_method_arg_types(method);
                    TypeScriptGenerator::instance().add_method_info(name, method, return_type,
                                                                    arg_types);
                } catch (...) {
                }
            }

            return *this;
        }

        template <typename Ret, typename... Args>
        EmGenerator &bind_function(const std::string &name, Ret (*func)(Args...)) {
            em::function(name.c_str(), func);

            std::vector<std::type_index> arg_types = {std::type_index(typeid(Args))...};
            TypeScriptGenerator::instance().add_function(name, std::type_index(typeid(Ret)),
                                                         arg_types);

            return *this;
        }

        EmGenerator &add_utilities() {
            struct Utils {
                static em::val listClasses() {
                    auto    classes = core::Registry::instance().list_classes();
                    em::val arr     = em::val::array();
                    for (size_t i = 0; i < classes.size(); ++i) {
                        arr.set(i, classes[i]);
                    }
                    return arr;
                }
                static std::string version() { return rosetta::version(); }
                static std::string generateTypeScript() {
                    return TypeScriptGenerator::instance().generate();
                }
                static em::val getInheritanceInfo(const std::string &class_name) {
                    em::val info = em::val::object();

                    auto    bases = InheritanceRegistry::instance().get_base_classes(class_name);
                    em::val bases_arr = em::val::array();
                    for (size_t i = 0; i < bases.size(); ++i) {
                        bases_arr.set(i, bases[i]);
                    }
                    info.set("bases", bases_arr);

                    auto derived = InheritanceRegistry::instance().get_derived_classes(class_name);
                    em::val derived_arr = em::val::array();
                    for (size_t i = 0; i < derived.size(); ++i) {
                        derived_arr.set(i, derived[i]);
                    }
                    info.set("derived", derived_arr);

                    auto ancestors = InheritanceRegistry::instance().get_all_ancestors(class_name);
                    em::val ancestors_arr = em::val::array();
                    for (size_t i = 0; i < ancestors.size(); ++i) {
                        ancestors_arr.set(i, ancestors[i]);
                    }
                    info.set("ancestors", ancestors_arr);

                    return info;
                }
            };
            em::function("listClasses", &Utils::listClasses);
            em::function("version", &Utils::version);
            em::function("generateTypeScript", &Utils::generateTypeScript);
            em::function("getInheritanceInfo", &Utils::getInheritanceInfo);
            return *this;
        }

        static std::string get_typescript_declarations() {
            return TypeScriptGenerator::instance().generate();
        }
    };

    inline EmGenerator create_bindings() {
        return EmGenerator();
    }

} // namespace rosetta::em_gen

// ============================================================================
// Macros
// ============================================================================

#define BEGIN_EM_MODULE(module_name)   \
    EMSCRIPTEN_BINDINGS(module_name) { \
        auto gen = rosetta::em_gen::create_bindings();

#define END_EM_MODULE() }

#define BIND_EM_CLASS(Class) gen.bind_class<Class>(#Class);
#define BIND_EM_CLASS_AUTO(Class, ...) gen.bind_class<Class, __VA_ARGS__>(#Class);
#define BIND_EM_CLASS_FACTORY(Class, factory) gen.bind_class_with_factory<Class>(factory, #Class);
#define BIND_EM_DERIVED_CLASS(Derived, Base, ...) \
    gen.bind_derived_class<Derived, Base, __VA_ARGS__>(#Derived);
#define BIND_EM_ABSTRACT_CLASS(Class) gen.bind_abstract_class<Class>(#Class);

#define BIND_EM_FUNCTION(func) gen.bind_function(#func, func);

#define BIND_EM_UTILITIES() gen.add_utilities();

// Register vector type converter for custom classes
#define BIND_EM_VECTOR_TYPE(Class) rosetta::em_gen::bind_vector_type<Class>();

/** Register function callback converter for std::function types Usage:
 * - `BIND_EM_FUNCTION_TYPE(Point,Point)` for `std::function<Point(Point)>`
 * - `BIND_EM_FUNCTION_TYPE(void, int, double)` for `std::function<void(int, double)>`
 */
#define BIND_EM_FUNCTION_TYPE(Ret, ...) rosetta::em_gen::bind_function_type<Ret, __VA_ARGS__>();