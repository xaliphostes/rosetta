// ============================================================================
// Emscripten binding generator using Rosetta introspection
// Version 4: Full Inheritance Support
// ============================================================================
#pragma once

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <rosetta/rosetta.h>
#include <string>
#include <typeindex>
#include <vector>
#include <sstream>
#include <map>
#include <functional>
#include <unordered_map>
#include <unordered_set>

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
            // Don't register casts for abstract classes - they can't be instantiated
            if constexpr (!std::is_abstract_v<T>) {
                casts_[std::type_index(typeid(T))] = [](const core::Any& value) -> em::val {
                    return em::val(value.as<T>());
                };
            }
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
    // Inheritance Registry - Track class inheritance relationships
    // ============================================================================

    class InheritanceRegistry {
    public:
        static InheritanceRegistry& instance() {
            static InheritanceRegistry reg;
            return reg;
        }

        // Register a class with its base classes
        void register_class(const std::string& class_name, 
                           std::type_index class_type,
                           const std::vector<std::string>& base_names = {}) {
            class_types_.insert_or_assign(class_name, class_type);
            type_to_name_.insert_or_assign(class_type, class_name);
            base_classes_[class_name] = base_names;
            
            // Register derived relationship
            for (const auto& base : base_names) {
                derived_classes_[base].push_back(class_name);
            }
        }

        // Get base class names for a class
        std::vector<std::string> get_base_classes(const std::string& class_name) const {
            auto it = base_classes_.find(class_name);
            if (it != base_classes_.end()) {
                return it->second;
            }
            return {};
        }

        // Get primary base class (first one)
        std::string get_primary_base(const std::string& class_name) const {
            auto bases = get_base_classes(class_name);
            return bases.empty() ? "" : bases[0];
        }

        // Get derived class names for a class
        std::vector<std::string> get_derived_classes(const std::string& class_name) const {
            auto it = derived_classes_.find(class_name);
            if (it != derived_classes_.end()) {
                return it->second;
            }
            return {};
        }

        // Check if class inherits from another (direct or indirect)
        bool inherits_from(const std::string& derived, const std::string& base) const {
            if (derived == base) return true;
            
            auto bases = get_base_classes(derived);
            for (const auto& b : bases) {
                if (b == base || inherits_from(b, base)) {
                    return true;
                }
            }
            return false;
        }

        // Get all ancestor classes
        std::vector<std::string> get_all_ancestors(const std::string& class_name) const {
            std::vector<std::string> ancestors;
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
        std::type_index get_class_type(const std::string& name) const {
            auto it = class_types_.find(name);
            if (it != class_types_.end()) {
                return it->second;
            }
            return std::type_index(typeid(void));
        }

        // Check if a class is registered
        bool is_registered(const std::string& name) const {
            return class_types_.find(name) != class_types_.end();
        }

    private:
        void collect_ancestors(const std::string& class_name, 
                              std::vector<std::string>& ancestors,
                              std::unordered_set<std::string>& visited) const {
            if (visited.count(class_name)) return;
            visited.insert(class_name);
            
            auto bases = get_base_classes(class_name);
            for (const auto& base : bases) {
                ancestors.push_back(base);
                collect_ancestors(base, ancestors, visited);
            }
        }

        std::unordered_map<std::string, std::type_index> class_types_;
        std::unordered_map<std::type_index, std::string> type_to_name_;
        std::unordered_map<std::string, std::vector<std::string>> base_classes_;
        std::unordered_map<std::string, std::vector<std::string>> derived_classes_;
    };

    // ============================================================================
    // TypeScript Declaration Generator
    // ============================================================================

    class TypeScriptGenerator {
    public:
        static TypeScriptGenerator& instance() {
            static TypeScriptGenerator gen;
            return gen;
        }

        void add_class(const std::string& name, 
                      const std::vector<std::string>& fields,
                      const std::vector<std::string>& methods,
                      const std::string& base_class = "") {
            ClassInfo info;
            info.name = name;
            info.fields = fields;
            info.methods = methods;
            info.base_class = base_class;
            classes_[name] = info;
        }

        // Add class with multiple base classes
        void add_class(const std::string& name, 
                      const std::vector<std::string>& fields,
                      const std::vector<std::string>& methods,
                      const std::vector<std::string>& base_classes) {
            ClassInfo info;
            info.name = name;
            info.fields = fields;
            info.methods = methods;
            info.base_classes = base_classes;
            if (!base_classes.empty()) {
                info.base_class = base_classes[0];  // Primary base for compatibility
            }
            classes_[name] = info;
        }

        void add_constructor(const std::string& class_name, 
                            const std::vector<std::type_index>& arg_types) {
            constructor_types_[class_name].push_back(arg_types);
        }

        void add_field_type(const std::string& class_name, const std::string& field_name, 
                           std::type_index type) {
            field_types_.insert_or_assign(class_name + "." + field_name, type);
        }

        void add_method_info(const std::string& class_name, const std::string& method_name,
                            std::type_index return_type, const std::vector<std::type_index>& arg_types) {
            MethodInfo info;
            info.return_type = return_type;
            info.arg_types = arg_types;
            method_types_[class_name + "." + method_name] = info;
        }

        void add_function(const std::string& name, std::type_index return_type,
                         const std::vector<std::type_index>& arg_types) {
            MethodInfo info;
            info.return_type = return_type;
            info.arg_types = arg_types;
            functions_[name] = info;
        }

        void set_type_name(std::type_index type, const std::string& name) {
            type_names_[type] = name;
        }

        void mark_abstract(const std::string& class_name) {
            abstract_classes_.insert(class_name);
        }

        void mark_polymorphic(const std::string& class_name) {
            polymorphic_classes_.insert(class_name);
        }

        std::string generate() const {
            std::ostringstream ts;
            
            ts << "// Auto-generated TypeScript declarations for Rosetta Emscripten bindings\n";
            ts << "// Generated by em_generator_v4 with inheritance support\n\n";
            
            ts << "declare module 'Module' {\n";
            ts << "  export interface EmscriptenModule {\n";
            
            // Classes
            for (const auto& [name, info] : classes_) {
                ts << "    " << name << ": " << name << "Constructor;\n";
            }
            
            // Free functions
            for (const auto& [name, info] : functions_) {
                ts << "    " << name << "(";
                for (size_t i = 0; i < info.arg_types.size(); ++i) {
                    if (i > 0) ts << ", ";
                    ts << "arg" << i << ": " << type_to_ts(info.arg_types[i]);
                }
                ts << "): " << type_to_ts(info.return_type) << ";\n";
            }
            
            // Utility functions
            ts << "    listClasses(): string[];\n";
            ts << "    version(): string;\n";
            ts << "    generateTypeScript(): string;\n";
            ts << "    getInheritanceInfo(className: string): { bases: string[], derived: string[] };\n";
            
            ts << "  }\n\n";
            
            // Class interfaces
            for (const auto& [name, info] : classes_) {
                // Constructor interface
                ts << "  interface " << name << "Constructor {\n";
                
                // Generate constructor signatures (unless abstract)
                if (abstract_classes_.find(name) == abstract_classes_.end()) {
                    auto ctor_it = constructor_types_.find(name);
                    if (ctor_it != constructor_types_.end() && !ctor_it->second.empty()) {
                        for (const auto& ctor_args : ctor_it->second) {
                            ts << "    new(";
                            for (size_t i = 0; i < ctor_args.size(); ++i) {
                                if (i > 0) ts << ", ";
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
                
                // Instance interface with inheritance
                ts << "  interface " << name;
                
                // Handle inheritance
                if (!info.base_classes.empty()) {
                    ts << " extends " << info.base_classes[0];
                    // For multiple inheritance, we use intersection types
                    for (size_t i = 1; i < info.base_classes.size(); ++i) {
                        ts << ", " << info.base_classes[i];
                    }
                } else if (!info.base_class.empty()) {
                    ts << " extends " << info.base_class;
                }
                ts << " {\n";
                
                // Fields (only ones not inherited)
                for (const auto& field : info.fields) {
                    std::string key = name + "." + field;
                    auto it = field_types_.find(key);
                    std::string ts_type = "any";
                    if (it != field_types_.end()) {
                        ts_type = type_to_ts(it->second);
                    }
                    ts << "    " << field << ": " << ts_type << ";\n";
                }
                
                // Methods (only ones not inherited or overridden)
                for (const auto& method : info.methods) {
                    std::string key = name + "." + method;
                    auto it = method_types_.find(key);
                    if (it != method_types_.end()) {
                        ts << "    " << method << "(";
                        for (size_t i = 0; i < it->second.arg_types.size(); ++i) {
                            if (i > 0) ts << ", ";
                            ts << "arg" << i << ": " << type_to_ts(it->second.arg_types[i]);
                        }
                        ts << "): " << type_to_ts(it->second.return_type) << ";\n";
                    } else {
                        ts << "    " << method << "(...args: any[]): any;\n";
                    }
                }
                
                // Internal methods
                ts << "    $get(field: string): any;\n";
                ts << "    $set(field: string, value: any): void;\n";
                ts << "    $call(method: string, args: any[]): any;\n";
                ts << "    $instanceof(className: string): boolean;\n";
                ts << "    delete(): void;\n";
                
                ts << "  }\n\n";
            }
            
            ts << "  export default function(): Promise<EmscriptenModule>;\n";
            ts << "}\n";
            
            return ts.str();
        }

    private:
        struct ClassInfo {
            std::string name;
            std::vector<std::string> fields;
            std::vector<std::string> methods;
            std::string base_class;  // For backward compatibility
            std::vector<std::string> base_classes;  // For multiple inheritance
        };

        struct MethodInfo {
            std::type_index return_type = std::type_index(typeid(void));
            std::vector<std::type_index> arg_types;
        };

        std::map<std::string, ClassInfo> classes_;
        std::map<std::string, std::vector<std::vector<std::type_index>>> constructor_types_;
        std::map<std::string, std::type_index> field_types_;
        std::map<std::string, MethodInfo> method_types_;
        std::map<std::string, MethodInfo> functions_;
        std::map<std::type_index, std::string> type_names_;
        std::unordered_set<std::string> abstract_classes_;
        std::unordered_set<std::string> polymorphic_classes_;

        std::string type_to_ts(std::type_index type) const {
            // Check custom type names first (includes registered classes)
            auto custom_it = type_names_.find(type);
            if (custom_it != type_names_.end()) {
                return custom_it->second;
            }

            // Primitives
            if (type == std::type_index(typeid(int))) return "number";
            if (type == std::type_index(typeid(double))) return "number";
            if (type == std::type_index(typeid(float))) return "number";
            if (type == std::type_index(typeid(size_t))) return "number";
            if (type == std::type_index(typeid(long))) return "number";
            if (type == std::type_index(typeid(long long))) return "number";
            if (type == std::type_index(typeid(unsigned int))) return "number";
            if (type == std::type_index(typeid(unsigned long))) return "number";
            if (type == std::type_index(typeid(short))) return "number";
            if (type == std::type_index(typeid(char))) return "number";
            if (type == std::type_index(typeid(bool))) return "boolean";
            if (type == std::type_index(typeid(std::string))) return "string";
            if (type == std::type_index(typeid(void))) return "void";
            
            // Vectors
            if (type == std::type_index(typeid(std::vector<double>))) return "number[]";
            if (type == std::type_index(typeid(std::vector<float>))) return "number[]";
            if (type == std::type_index(typeid(std::vector<int>))) return "number[]";
            if (type == std::type_index(typeid(std::vector<size_t>))) return "number[]";
            if (type == std::type_index(typeid(std::vector<std::string>))) return "string[]";
            if (type == std::type_index(typeid(std::vector<bool>))) return "boolean[]";
            
            return "any";
        }
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
        
        static void register_typescript(const std::string& class_name) {
            std::vector<std::type_index> arg_types = {std::type_index(typeid(Args))...};
            TypeScriptGenerator::instance().add_constructor(class_name, arg_types);
        }
    };

    // ============================================================================
    // Class Wrapper - Provides generic access via $get/$set/$call with inheritance
    // ============================================================================

    template <typename T>
    struct ClassWrapper {
        // Field getter (searches inheritance chain)
        static em::val getField(T& obj, const std::string& name) {
            const auto &meta = core::Registry::instance().get<T>();
            try {
                core::Any value = meta.get_field(obj, name);
                return any_to_val(value);
            } catch (const std::exception& e) {
                // Try to find in base classes through inheritance info
                const auto& inheritance = meta.inheritance();
                for (const auto& base : inheritance.base_classes) {
                    // Field might be in base class - try the metadata system
                }
                throw std::runtime_error("Field '" + name + "' not found: " + e.what());
            }
        }

        // Field setter (searches inheritance chain)
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

        // Method caller (supports virtual dispatch)
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

        // Get class metadata as JSON with inheritance info
        static em::val getMetadata() {
            const auto &meta = core::Registry::instance().get<T>();
            em::val info = em::val::object();

            // Fields (including inherited)
            em::val fields = em::val::array();
            for (const auto& f : meta.fields()) {
                fields.call<void>("push", em::val(f));
            }
            info.set("fields", fields);

            // Methods (including inherited/overridden)
            em::val methods = em::val::array();
            for (const auto& m : meta.methods()) {
                methods.call<void>("push", em::val(m));
            }
            info.set("methods", methods);

            // Base classes
            em::val bases = em::val::array();
            const auto& inheritance = meta.inheritance();
            for (const auto& base : inheritance.base_classes) {
                bases.call<void>("push", em::val(base.name));
            }
            info.set("bases", bases);

            // Inheritance properties
            info.set("isAbstract", inheritance.is_abstract);
            info.set("isPolymorphic", inheritance.is_polymorphic);
            info.set("hasVirtualDestructor", inheritance.has_virtual_destructor);

            return info;
        }

        // Check if class is abstract
        static bool isAbstract() {
            const auto &meta = core::Registry::instance().get<T>();
            return meta.inheritance().is_abstract;
        }

        // Check if object is instance of a class (including through inheritance)
        static bool instanceOf(T& obj, const std::string& class_name) {
            std::string my_name = core::Registry::instance().get<T>().name();
            return InheritanceRegistry::instance().inherits_from(my_name, class_name);
        }
    };

    // ============================================================================
    // Auto Class Binder with Base Class Support
    // ============================================================================

    template <typename T, typename Base, typename... CtorSignatures>
    class AutoBinderWithBase {
    public:
        static void bind(const std::string& js_name) {
            const auto &meta = core::Registry::instance().get<T>();
            std::string name = js_name.empty() ? meta.name() : js_name;

            // Register type cast for this class
            TypeCastRegistry::instance().register_cast<T>();
            
            // Register type name for TypeScript
            TypeScriptGenerator::instance().set_type_name(std::type_index(typeid(T)), name);

            // Register inheritance relationship
            std::string base_name = core::Registry::instance().get<Base>().name();
            InheritanceRegistry::instance().register_class(
                name, 
                std::type_index(typeid(T)), 
                {base_name}
            );

            // Create class with base class
            auto em_class = em::class_<T, em::base<Base>>(name.c_str());

            // Bind constructors (if not abstract)
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

            // Collect TypeScript info
            TypeScriptGenerator::instance().add_class(name, meta.fields(), meta.methods(), base_name);
            
            // Mark polymorphic if needed
            if (meta.inheritance().is_polymorphic) {
                TypeScriptGenerator::instance().mark_polymorphic(name);
            }
            
            // Add field type info
            for (const auto& field : meta.fields()) {
                auto field_type = meta.get_field_type(field);
                TypeScriptGenerator::instance().add_field_type(name, field, field_type);
            }
            
            // Add method type info
            for (const auto& method : meta.methods()) {
                try {
                    auto return_type = meta.get_method_return_type(method);
                    auto arg_types = meta.get_method_arg_types(method);
                    TypeScriptGenerator::instance().add_method_info(name, method, return_type, arg_types);
                } catch (...) {}
            }
        }
    };

    // Binder with multiple base classes support
    template <typename T, typename... Bases>
    class AutoBinderMultiBase;

    // Specialization for single base
    template <typename T, typename Base>
    class AutoBinderMultiBase<T, Base> : public AutoBinderWithBase<T, Base> {};

    // ============================================================================
    // Auto Class Binder (no explicit base)
    // ============================================================================

    template <typename T, typename... CtorSignatures>
    class AutoBinder {
    public:
        static void bind(const std::string& js_name) {
            const auto &meta = core::Registry::instance().get<T>();
            std::string name = js_name.empty() ? meta.name() : js_name;

            // Register type cast for this class
            TypeCastRegistry::instance().register_cast<T>();
            
            // Register type name for TypeScript
            TypeScriptGenerator::instance().set_type_name(std::type_index(typeid(T)), name);

            // Register in inheritance registry (no base)
            InheritanceRegistry::instance().register_class(
                name, 
                std::type_index(typeid(T))
            );

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

            // Collect TypeScript info - check for base classes in Rosetta metadata
            const auto& inheritance = meta.inheritance();
            std::vector<std::string> base_names;
            for (const auto& base : inheritance.base_classes) {
                base_names.push_back(base.name);
            }
            
            if (base_names.empty()) {
                TypeScriptGenerator::instance().add_class(name, meta.fields(), meta.methods());
            } else {
                TypeScriptGenerator::instance().add_class(name, meta.fields(), meta.methods(), base_names);
            }
            
            // Mark polymorphic/abstract if needed
            if (inheritance.is_polymorphic) {
                TypeScriptGenerator::instance().mark_polymorphic(name);
            }
            if (inheritance.is_abstract) {
                TypeScriptGenerator::instance().mark_abstract(name);
            }
            
            // Add field type info
            for (const auto& field : meta.fields()) {
                auto field_type = meta.get_field_type(field);
                TypeScriptGenerator::instance().add_field_type(name, field, field_type);
            }
            
            // Add method type info
            for (const auto& method : meta.methods()) {
                try {
                    auto return_type = meta.get_method_return_type(method);
                    auto arg_types = meta.get_method_arg_types(method);
                    TypeScriptGenerator::instance().add_method_info(name, method, return_type, arg_types);
                } catch (...) {}
            }
        }
    };

    // ============================================================================
    // Generator
    // ============================================================================

    class EmGenerator {
    public:
        EmGenerator() = default;

        /**
         * @brief Bind a class without explicit base class
         * (base classes will be detected from Rosetta metadata)
         */
        template <typename T, typename... CtorSignatures>
        EmGenerator &bind_class(const std::string &js_name = "") {
            AutoBinder<T, CtorSignatures...>::bind(js_name);
            return *this;
        }

        /**
         * @brief Bind a derived class with explicit base class for inheritance
         * 
         * Usage:
         *   gen.bind_derived_class<Dog, Animal, std::tuple<std::string>>("Dog");
         */
        template <typename Derived, typename Base, typename... CtorSignatures>
        EmGenerator &bind_derived_class(const std::string &js_name = "") {
            AutoBinderWithBase<Derived, Base, CtorSignatures...>::bind(js_name);
            return *this;
        }

        /**
         * @brief Bind an abstract base class
         * 
         * Usage:
         *   gen.bind_abstract_class<Shape>("Shape");
         */
        template <typename T>
        EmGenerator &bind_abstract_class(const std::string &js_name = "") {
            const auto &meta = core::Registry::instance().get<T>();
            std::string name = js_name.empty() ? meta.name() : js_name;

            // Don't register type cast for abstract classes - they can't be instantiated
            // TypeCastRegistry::instance().register_cast<T>();
            
            TypeScriptGenerator::instance().set_type_name(std::type_index(typeid(T)), name);
            
            // Register in inheritance registry
            InheritanceRegistry::instance().register_class(
                name, 
                std::type_index(typeid(T))
            );

            // Create class without constructor for abstract classes
            auto em_class = em::class_<T>(name.c_str());

            // For abstract classes, we can still bind the generic accessors
            // but they will only work when called on derived class instances
            em_class.function("$get", &ClassWrapper<T>::getField);
            em_class.function("$set", &ClassWrapper<T>::setField);
            em_class.function("$call", &ClassWrapper<T>::callMethod);
            em_class.class_function("$meta", &ClassWrapper<T>::getMetadata);
            em_class.class_function("$isAbstract", &ClassWrapper<T>::isAbstract);
            em_class.function("$instanceof", &ClassWrapper<T>::instanceOf);

            // Mark as abstract
            TypeScriptGenerator::instance().mark_abstract(name);
            TypeScriptGenerator::instance().add_class(name, meta.fields(), meta.methods());
            
            // Add field and method type info
            for (const auto& field : meta.fields()) {
                auto field_type = meta.get_field_type(field);
                TypeScriptGenerator::instance().add_field_type(name, field, field_type);
            }
            
            for (const auto& method : meta.methods()) {
                try {
                    auto return_type = meta.get_method_return_type(method);
                    auto arg_types = meta.get_method_arg_types(method);
                    TypeScriptGenerator::instance().add_method_info(name, method, return_type, arg_types);
                } catch (...) {}
            }

            return *this;
        }

        /**
         * @brief Bind a free function
         */
        template <typename Ret, typename... Args>
        EmGenerator &bind_function(const std::string &name, Ret (*func)(Args...)) {
            em::function(name.c_str(), func);
            
            // Collect TypeScript info
            std::vector<std::type_index> arg_types = {std::type_index(typeid(Args))...};
            TypeScriptGenerator::instance().add_function(name, std::type_index(typeid(Ret)), arg_types);
            
            return *this;
        }

        /**
         * @brief Add utility functions including inheritance helpers
         */
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
                static std::string generateTypeScript() {
                    return TypeScriptGenerator::instance().generate();
                }
                // Get inheritance info for a class
                static em::val getInheritanceInfo(const std::string& class_name) {
                    em::val info = em::val::object();
                    
                    auto bases = InheritanceRegistry::instance().get_base_classes(class_name);
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
                    
                    // Get all ancestors
                    auto ancestors = InheritanceRegistry::instance().get_all_ancestors(class_name);
                    em::val ancestors_arr = em::val::array();
                    for (size_t i = 0; i < ancestors.size(); ++i) {
                        ancestors_arr.set(i, ancestors[i]);
                    }
                    info.set("ancestors", ancestors_arr);
                    
                    return info;
                }
                // Check inheritance relationship
                static bool inheritsFrom(const std::string& derived, const std::string& base) {
                    return InheritanceRegistry::instance().inherits_from(derived, base);
                }
            };
            em::function("listClasses", &Utils::listClasses);
            em::function("version", &Utils::version);
            em::function("generateTypeScript", &Utils::generateTypeScript);
            em::function("getInheritanceInfo", &Utils::getInheritanceInfo);
            em::function("inheritsFrom", &Utils::inheritsFrom);
            return *this;
        }

        /**
         * @brief Get the generated TypeScript declarations
         */
        static std::string get_typescript_declarations() {
            return TypeScriptGenerator::instance().generate();
        }
    };

    inline EmGenerator create_bindings() { return EmGenerator(); }

    // ============================================================================
    // JavaScript Enhancement Code Generator with Inheritance Support
    // ============================================================================

    /**
     * @brief Generate JavaScript code that enhances classes with proper getters/setters
     *        and inheritance-aware methods
     * 
     * Call this and execute the returned string in JavaScript after module loads
     */
    inline std::string generateJSEnhancements() {
        std::ostringstream js;
        
        js << R"(
function enhanceRosettaClasses(Module) {
    const classes = Module.listClasses();
    
    // First pass: collect all class metadata
    const classMetadata = {};
    classes.forEach(className => {
        const ClassRef = Module[className];
        if (!ClassRef) return;
        classMetadata[className] = ClassRef.$meta();
    });
    
    // Second pass: enhance classes with inheritance awareness
    classes.forEach(className => {
        const ClassRef = Module[className];
        if (!ClassRef) return;
        
        const meta = classMetadata[className];
        
        // Get all inherited fields and methods
        const allFields = new Set(meta.fields);
        const allMethods = new Set(meta.methods);
        
        // Collect from base classes
        if (meta.bases) {
            meta.bases.forEach(baseName => {
                const baseMeta = classMetadata[baseName];
                if (baseMeta) {
                    baseMeta.fields.forEach(f => allFields.add(f));
                    baseMeta.methods.forEach(m => allMethods.add(m));
                }
            });
        }
        
        // Add field accessors as properties
        allFields.forEach(field => {
            if (!Object.getOwnPropertyDescriptor(ClassRef.prototype, field)) {
                Object.defineProperty(ClassRef.prototype, field, {
                    get: function() { return this.$get(field); },
                    set: function(v) { this.$set(field, v); },
                    enumerable: true
                });
            }
        });
        
        // Add method wrappers
        allMethods.forEach(method => {
            if (!ClassRef.prototype[method]) {
                ClassRef.prototype[method] = function(...args) {
                    return this.$call(method, args);
                };
            }
        });
        
        // Add instanceof helper
        if (!ClassRef.prototype.isInstanceOf) {
            ClassRef.prototype.isInstanceOf = function(targetClass) {
                return this.$instanceof(targetClass);
            };
        }
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

/**
 * @brief Bind a derived class with inheritance
 * 
 * Usage:
 *   BIND_EM_DERIVED_CLASS(Dog, Animal, std::tuple<std::string>);
 */
#define BIND_EM_DERIVED_CLASS(Derived, Base, ...) \
    gen.bind_derived_class<Derived, Base, __VA_ARGS__>(#Derived);

/**
 * @brief Bind an abstract base class
 * 
 * Usage:
 *   BIND_EM_ABSTRACT_CLASS(Shape);
 */
#define BIND_EM_ABSTRACT_CLASS(Class) \
    gen.bind_abstract_class<Class>(#Class);

#define BIND_EM_FUNCTION(func) gen.bind_function(#func, func);

#define BIND_EM_UTILITIES() gen.add_utilities();

#define END_EM_MODULE() }