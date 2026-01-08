// ============================================================================
// Class metadata for Rosetta introspection system - FIXED FOR OVERLOADS
// ============================================================================
#pragma once
#include "any.h"
#include "demangler.h"
#include "inheritance_info.h"
#include "virtual_method_registry.h"
#include <functional>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rosetta::core {

    // ============================================================================
    // PropertyInfo - Information about a registered property (virtual field)
    // ============================================================================
    struct PropertyInfo {
        std::string     name;        // Property name (e.g., "points")
        std::string     getter_name; // Original getter method name (may be empty)
        std::string     setter_name; // Original setter method name (may be empty)
        std::type_index value_type;  // Type of the property value
        bool            is_readonly  = false;
        bool            is_writeonly = false;
        std::string     doc;         // User documentation for this property

        PropertyInfo() : value_type(typeid(void)) {}
        PropertyInfo(const std::string &n, std::type_index t) : name(n), value_type(t) {}
    };

    // ============================================================================
    // ClassMetadata
    // ============================================================================

    /**
     * @brief MetaData of a class. This class allows to register fields, methods, constructors, etc.
     * to introspect a any class at runtime.
     * @tparam Class Type of the class
     */
    template <typename Class> class ClassMetadata {
        std::string     name_;
        InheritanceInfo inheritance_;

    public:
        template <typename T> using umap  = std::unordered_map<std::string, T>;
        template <typename T> using umapv = std::unordered_map<std::string, std::vector<T>>;

    private:
        umap<std::function<Any(Class &)>>       field_getters_;
        umap<std::function<void(Class &, Any)>> field_setters_;

        // ========================================================================
        // Property storage (virtual fields registered via .property())
        // ========================================================================
        umap<std::function<Any(Class &)>>       property_getters_;
        umap<std::function<void(Class &, Any)>> property_setters_;
        std::vector<std::string>                property_names_;
        umap<PropertyInfo>                      property_info_;

        // Store multiple invokers per method name to support overloads
        umapv<std::function<Any(Class &, std::vector<Any>)>>       methods_;
        umapv<std::function<Any(const Class &, std::vector<Any>)>> const_methods_;

        // ----------------------------------------

        using Constructor = std::function<Any(std::vector<Any>)>;
        std::vector<Constructor> constructors_;

        struct ConstructorInfo {
            std::function<Any(std::vector<Any>)> invoker;
            std::vector<std::type_index>         param_types;
            std::vector<bool>                    param_is_lvalue_ref; // true if param is T&
            size_t                               arity     = 0;
            bool                                 is_lambda = false;
            std::string                          lambda_body; // For code generation
            std::string                          doc;         // User documentation
        };
        std::vector<ConstructorInfo> constructor_infos_;

        // ----------------------------------------

        std::vector<std::string> field_names_;
        // Store type information for fields (for JS binding)
        umap<std::type_index> field_types_;

        // ----------------------------------------

        std::vector<std::string> method_names_;

    public:
        struct MethodInfo {
            std::function<Any(Class &, std::vector<Any>)> invoker;
            std::vector<std::type_index>                  arg_types;
            std::type_index return_type = std::type_index(typeid(void));
            size_t          arity       = 0;
            bool            is_static   = false; // Flag to identify static methods
            std::string     inherited_from; // Empty if not inherited, base class name otherwise

            bool        is_const      = false;
            bool        is_overloaded = false;
            bool        is_lambda     = false;
            std::string doc;          // User documentation
        };

    private:
        // Store multiple MethodInfo per method name to support overloads
        umapv<MethodInfo> method_info_;

        // ----------------------------------------
        // Static methods storage (no object instance needed)
        // ----------------------------------------

        struct StaticMethodInfo {
            std::function<Any(std::vector<Any>)> invoker;
            std::vector<std::type_index>         arg_types;
            std::type_index                      return_type = std::type_index(typeid(void));
            size_t                               arity       = 0;
            std::string                          doc;        // User documentation
        };
        // Store multiple StaticMethodInfo per method name to support overloads
        umapv<StaticMethodInfo> static_methods_;

        // ----------------------------------------
        // Documentation storage
        // ----------------------------------------
        std::string         class_doc_;  // Documentation for the class itself
        umap<std::string>   field_docs_; // Documentation for each field

        // ----------------------------------------

    public:
        // ========================================================================
        // DocProxy - Enables fluent .doc() chaining after registration calls
        // ========================================================================

        enum class DocTarget { Klass, Field, Method, Property, Constructor, StaticMethod };

        class DocProxy {
            ClassMetadata &metadata_;
            DocTarget      target_;
            std::string    element_name_;  // For field/method/property
            size_t         element_index_; // For constructor (index in vector)

        public:
            DocProxy(ClassMetadata &m, DocTarget t, std::string name = "", size_t idx = 0)
                : metadata_(m), target_(t), element_name_(std::move(name)), element_index_(idx) {}

            /// Set documentation for the last registered element
            DocProxy doc(const std::string &documentation) {
                switch (target_) {
                    case DocTarget::Klass:
                        metadata_.class_doc_ = documentation;
                        break;
                    case DocTarget::Field:
                        metadata_.field_docs_[element_name_] = documentation;
                        break;
                    case DocTarget::Method:
                        // Set doc on all overloads with this name (last added)
                        if (auto it = metadata_.method_info_.find(element_name_);
                            it != metadata_.method_info_.end() && !it->second.empty()) {
                            it->second.back().doc = documentation;
                        }
                        break;
                    case DocTarget::Property:
                        if (auto it = metadata_.property_info_.find(element_name_);
                            it != metadata_.property_info_.end()) {
                            it->second.doc = documentation;
                        }
                        break;
                    case DocTarget::Constructor:
                        if (element_index_ < metadata_.constructor_infos_.size()) {
                            metadata_.constructor_infos_[element_index_].doc = documentation;
                        }
                        break;
                    case DocTarget::StaticMethod:
                        if (auto it = metadata_.static_methods_.find(element_name_);
                            it != metadata_.static_methods_.end() && !it->second.empty()) {
                            it->second.back().doc = documentation;
                        }
                        break;
                }
                return *this;
            }

            /// Implicit conversion to ClassMetadata& for continued chaining
            operator ClassMetadata &() { return metadata_; }

            // ================================================================
            // Forwarding methods for chaining support
            // These allow continued registration after .doc() or without it
            // ================================================================

            template <typename... Args>
            DocProxy constructor() { return metadata_.template constructor<Args...>(); }

            template <typename... Args>
            DocProxy lambda_constructor(const std::string &lambda_body) {
                return metadata_.template lambda_constructor<Args...>(lambda_body);
            }

            template <typename T>
            DocProxy field(const std::string &name, T Class::*ptr) {
                return metadata_.field(name, ptr);
            }

            template <typename Base, typename T>
            DocProxy base_field(const std::string &name, T Base::*ptr) {
                return metadata_.template base_field<Base>(name, ptr);
            }

            template <typename T>
            DocProxy property(const std::string &name, const T &(Class::*getter)() const,
                              void (Class::*setter)(const T &)) {
                return metadata_.property(name, getter, setter);
            }

            template <typename T>
            DocProxy property(const std::string &name, T (Class::*getter)() const,
                              void (Class::*setter)(const T &)) {
                return metadata_.property(name, getter, setter);
            }

            template <typename T>
            DocProxy property(const std::string &name, T &(Class::*getter)(),
                              void (Class::*setter)(const T &)) {
                return metadata_.property(name, getter, setter);
            }

            template <typename T>
            DocProxy property(const std::string &name, const T &(Class::*getter)() const,
                              void (Class::*setter)(T)) {
                return metadata_.property(name, getter, setter);
            }

            template <typename T>
            DocProxy property(const std::string &name, T (Class::*getter)() const,
                              void (Class::*setter)(T)) {
                return metadata_.property(name, getter, setter);
            }

            template <typename T>
            DocProxy property(const std::string &name, T &(Class::*getter)(),
                              void (Class::*setter)(T)) {
                return metadata_.property(name, getter, setter);
            }

            template <typename T>
            DocProxy readonly_property(const std::string &name, const T &(Class::*getter)() const) {
                return metadata_.readonly_property(name, getter);
            }

            template <typename T>
            DocProxy readonly_property(const std::string &name, T (Class::*getter)() const) {
                return metadata_.readonly_property(name, getter);
            }

            template <typename T>
            DocProxy writeonly_property(const std::string &name, void (Class::*setter)(const T &)) {
                return metadata_.writeonly_property(name, setter);
            }

            template <typename Ret, typename... Args>
            DocProxy method(const std::string &name, Ret (Class::*ptr)(Args...)) {
                return metadata_.method(name, ptr);
            }

            template <typename Ret, typename... Args>
            DocProxy method(const std::string &name, Ret (Class::*ptr)(Args...) const) {
                return metadata_.method(name, ptr);
            }

            template <typename Ret, typename... Args>
            DocProxy overloaded_method(const std::string &name, Ret (Class::*ptr)(Args...)) {
                return metadata_.overloaded_method(name, ptr);
            }

            template <typename Ret, typename... Args>
            DocProxy overloaded_method(const std::string &name, Ret (Class::*ptr)(Args...) const) {
                return metadata_.overloaded_method(name, ptr);
            }

            template <typename Ret, typename... Args>
            DocProxy static_method(const std::string &name, Ret (*ptr)(Args...)) {
                return metadata_.static_method(name, ptr);
            }

            template <typename Base, typename Ret, typename... Args>
            DocProxy base_method(const std::string &name, Ret (Base::*ptr)(Args...)) {
                return metadata_.template base_method<Base>(name, ptr);
            }

            template <typename Base, typename Ret, typename... Args>
            DocProxy base_method(const std::string &name, Ret (Base::*ptr)(Args...) const) {
                return metadata_.template base_method<Base>(name, ptr);
            }

            template <typename Ret, typename... Args>
            DocProxy virtual_method(const std::string &name, Ret (Class::*ptr)(Args...)) {
                return metadata_.virtual_method(name, ptr);
            }

            template <typename Ret, typename... Args>
            DocProxy virtual_method(const std::string &name, Ret (Class::*ptr)(Args...) const) {
                return metadata_.virtual_method(name, ptr);
            }

            template <typename Ret, typename... Args>
            DocProxy pure_virtual_method(const std::string &name) {
                return metadata_.template pure_virtual_method<Ret, Args...>(name);
            }

            template <typename Ret, typename... Args>
            DocProxy pure_virtual_method_const(const std::string &name) {
                return metadata_.template pure_virtual_method_const<Ret, Args...>(name);
            }

            template <typename Ret, typename... Args>
            DocProxy override_method(const std::string &name, Ret (Class::*ptr)(Args...)) {
                return metadata_.override_method(name, ptr);
            }

            template <typename Ret, typename... Args>
            DocProxy override_method(const std::string &name, Ret (Class::*ptr)(Args...) const) {
                return metadata_.override_method(name, ptr);
            }

            template <typename Ret, typename... Args, typename Callable>
            DocProxy lambda_method(const std::string &name, Callable &&callable) {
                return metadata_.template lambda_method<Ret, Args...>(name, std::forward<Callable>(callable));
            }

            template <typename Ret, typename... Args, typename Callable>
            DocProxy lambda_method_const(const std::string &name, Callable &&callable) {
                return metadata_.template lambda_method_const<Ret, Args...>(name, std::forward<Callable>(callable));
            }

            template <typename Base>
            DocProxy inherits_from(const std::string &base_name = "", AccessSpecifier access = AccessSpecifier::Public) {
                metadata_.template inherits_from<Base>(base_name, access);
                return *this;
            }

            template <typename Base>
            DocProxy virtually_inherits_from(const std::string &base_name = "", AccessSpecifier access = AccessSpecifier::Public) {
                metadata_.template virtually_inherits_from<Base>(base_name, access);
                return *this;
            }
        };

    public:
        /**
         * @brief Constructeur
         * @param name Nom de la classe
         */
        explicit ClassMetadata(std::string name);

        /**
         * @brief Generic dumper for any registered class
         */
        void dump(std::ostream &os) const;

        /**
         * @brief Obtient le nom de la classe
         */
        const std::string &name() const;

        // ========================================================================
        // Documentation methods
        // ========================================================================

        /**
         * @brief Set documentation for this class
         * @param documentation The class documentation string
         * @return DocProxy for continued chaining
         */
        DocProxy doc(const std::string &documentation) {
            class_doc_ = documentation;
            return DocProxy(*this, DocTarget::Klass);
        }

        /**
         * @brief Get documentation for this class
         */
        const std::string &class_doc() const { return class_doc_; }

        /**
         * @brief Get documentation for a field
         * @param name Field name
         * @return Documentation string (empty if not set)
         */
        std::string get_field_doc(const std::string &name) const {
            auto it = field_docs_.find(name);
            return it != field_docs_.end() ? it->second : "";
        }

        /**
         * @brief Get documentation for a method (returns doc of first overload)
         * @param name Method name
         * @return Documentation string (empty if not set)
         */
        std::string get_method_doc(const std::string &name) const {
            auto it = method_info_.find(name);
            if (it != method_info_.end() && !it->second.empty()) {
                return it->second.front().doc;
            }
            return "";
        }

        /**
         * @brief Get documentation for a property
         * @param name Property name
         * @return Documentation string (empty if not set)
         */
        std::string get_property_doc(const std::string &name) const {
            auto it = property_info_.find(name);
            return it != property_info_.end() ? it->second.doc : "";
        }

        /**
         * @brief Get documentation for a constructor by index
         * @param index Constructor index
         * @return Documentation string (empty if not set or out of range)
         */
        std::string get_constructor_doc(size_t index) const {
            return index < constructor_infos_.size() ? constructor_infos_[index].doc : "";
        }

        /**
         * @brief Get inheritance information (const version)
         */
        const InheritanceInfo &inheritance() const;

        /**
         * @brief Get inheritance information (non-const version)
         */
        InheritanceInfo &inheritance();

        // ========================================================================
        // Virtual field registration (properties with getter/setter)
        // ========================================================================

        /**
         * @brief Register a virtual field using getter and setter methods
         * @tparam T Type of the field
         * @param name Name of the virtual field
         * @param getter Pointer to const getter method (returns by const reference)
         * @param setter Pointer to setter method
         */
        template <typename T>
        DocProxy property(const std::string &name, const T &(Class::*getter)() const,
                          void (Class::*setter)(const T &));

        /**
         * @brief Register a virtual field - variant with getter returning by value
         */
        template <typename T>
        DocProxy property(const std::string &name, T (Class::*getter)() const,
                          void (Class::*setter)(const T &));

        /**
         * @brief Register a virtual field - variant with non-const reference getter
         */
        template <typename T>
        DocProxy property(const std::string &name, T &(Class::*getter)(),
                          void (Class::*setter)(const T &));

        /**
         * @brief Register a virtual field - setter takes value by value (not const ref)
         */
        template <typename T>
        DocProxy property(const std::string &name, const T &(Class::*getter)() const,
                          void (Class::*setter)(T));

        /**
         * @brief Register a virtual field - getter by value, setter by value
         */
        template <typename T>
        DocProxy property(const std::string &name, T (Class::*getter)() const,
                          void (Class::*setter)(T));

        /**
         * @brief Register a virtual field - non-const ref getter, setter by value
         */
        template <typename T>
        DocProxy property(const std::string &name, T &(Class::*getter)(),
                          void (Class::*setter)(T));

        /**
         * @brief Register a read-only virtual field (getter only, no setter)
         */
        template <typename T>
        DocProxy readonly_property(const std::string &name,
                                   const T &(Class::*getter)() const);

        /**
         * @brief Register a read-only virtual field - variant returning by value
         */
        template <typename T>
        DocProxy readonly_property(const std::string &name, T (Class::*getter)() const);

        /**
         * @brief Register a write-only virtual field (setter only, no getter)
         */
        template <typename T>
        DocProxy writeonly_property(const std::string &name,
                                    void (Class::*setter)(const T &));

        // ========================================================================
        // Declare inheritance
        // ========================================================================

        /**
         * @brief Tells that this class inherits from a base class
         * @tparam Base Type of the base class
         * @param base_name Name of the base (optional)
         * @param access Access specifier
         */
        template <typename Base>
        ClassMetadata &inherits_from(const std::string &base_name = "",
                                     AccessSpecifier    access    = AccessSpecifier::Public);

        /**
         * @brief Tells that this class virtually inherits from a base class
         * @tparam Base Type of the base class
         * @param base_name Name of the base (optional)
         * @param access Access specifier
         */
        template <typename Base>
        ClassMetadata &virtually_inherits_from(const std::string &base_name = "",
                                               AccessSpecifier    access = AccessSpecifier::Public);

        // ========================================================================
        // Constructor registration
        // ========================================================================

        /**
         * @brief Register a constructor
         */
        template <typename... Args> DocProxy constructor();

        /**
         * @brief Register a lambda constructor for custom object creation.
         *
         * This allows binding a constructor with custom type conversions.
         * The lambda receives the bound parameters and returns a Class instance.
         *
         * Usage:
         *   .lambda_constructor<std::function<std::vector<double>(double, double, double)>, bool>(
         *       "[](std::function<std::vector<double>(double, double, double)> userFct, bool isStress) {"
         *       "    auto wrapper = [userFct](double x, double y, double z) -> Matrix33 {"
         *       "        auto v = userFct(x, y, z);"
         *       "        return Matrix33(v[0], v[1], v[2], v[3], v[4], v[5]);"
         *       "    };"
         *       "    return UserRemote(wrapper, isStress);"
         *       "}"
         *   );
         *
         * @tparam Args Parameter types that the binding will accept
         * @param lambda_body The lambda body as a string for code generation
         */
        template <typename... Args>
        DocProxy lambda_constructor(const std::string &lambda_body);

        // ========================================================================
        // Register lambda/callable as methods (synthetic methods)
        // ========================================================================

        /**
         * @brief Register a lambda or callable as a non-const method.
         *
         * The lambda's first parameter should be Class& (the "self" parameter).
         * This allows adding methods that don't exist in the original class.
         *
         * Usage:
         *   .lambda_method<void, double, int>("runCustom",
         *       [](Seidel& self, double tol, int maxiter) {
         *           self.setTolerance(tol);
         *           self.run();
         *       });
         *
         * @tparam Ret      Return type of the method
         * @tparam Args     Parameter types (excluding the self parameter)
         * @tparam Callable Type of the callable (auto-deduced)
         * @param name      Name of the method
         * @param callable  The lambda or callable object
         */
        template <typename Ret, typename... Args, typename Callable>
        DocProxy lambda_method(const std::string &name, Callable &&callable);

        /**
         * @brief Register a lambda or callable as a const method.
         *
         * The lambda's first parameter should be const Class& (the "self" parameter).
         *
         * @tparam Ret      Return type of the method
         * @tparam Args     Parameter types (excluding the self parameter)
         * @tparam Callable Type of the callable (auto-deduced)
         * @param name      Name of the method
         * @param callable  The lambda or callable object
         */
        template <typename Ret, typename... Args, typename Callable>
        DocProxy lambda_method_const(const std::string &name, Callable &&callable);

    private:
        // Helper to construct with proper indexing
        template <typename... Args, std::size_t... Is>
        static Any construct_with_indices(const std::vector<Any> &args, std::index_sequence<Is...>);

        // Helper to invoke lambda with Class& as first argument
        template <typename Ret, typename... Args, typename Callable, size_t... Is>
        static Any invoke_lambda(Callable &callable, Class &obj, std::vector<Any> &args,
                                 std::index_sequence<Is...>);

        // Helper to invoke lambda with const Class& as first argument
        template <typename Ret, typename... Args, typename Callable, size_t... Is>
        static Any invoke_lambda_const(Callable &callable, const Class &obj, std::vector<Any> &args,
                                       std::index_sequence<Is...>);

    public:
        // ========================================================================
        // Field registration
        // ========================================================================

        /**
         * @brief Register a field
         * @tparam T Type of the field
         * @param name Name of the field
         * @param ptr Pointer to member field
         */
        template <typename T> DocProxy field(const std::string &name, T Class::*ptr);

        /**
         * @brief Enregistre un champ d'une classe de base
         * @tparam Base Type de la classe de base
         * @tparam T Type du champ
         * @param name Nom du champ
         * @param ptr Pointeur vers membre de la base
         */
        template <typename Base, typename T>
        DocProxy base_field(const std::string &name, T Base::*ptr);

        // ========================================================================
        // Register non const methods
        // ========================================================================

        /**
         * @brief Register non-const method
         * @tparam Ret return type
         * @tparam Args Types of parameters
         * @param name Name of the method
         * @param ptr Pointer to the method
         */
        template <typename Ret, typename... Args>
        DocProxy method(const std::string &name, Ret (Class::*ptr)(Args...));

        /**
         * @brief Register an overloaded non-const method (disambiguated with explicit signature)
         * Use this with ROSETTA_OVERLOAD macro for methods that have multiple overloads
         */
        template <typename Ret, typename... Args>
        DocProxy overloaded_method(const std::string &name, Ret (Class::*ptr)(Args...));

        // ========================================================================
        // Register const methods
        // ========================================================================

        /**
         * @brief Register const method
         */
        template <typename Ret, typename... Args>
        DocProxy method(const std::string &name, Ret (Class::*ptr)(Args...) const);

        /**
         * @brief Register an overloaded const method (disambiguated with explicit signature)
         * Use this with ROSETTA_OVERLOAD_CONST macro
         */
        template <typename Ret, typename... Args>
        DocProxy overloaded_method(const std::string &name, Ret (Class::*ptr)(Args...) const);

        // ========================================================================
        // Register static methods
        // ========================================================================

        /**
         * @brief Register static method
         * @tparam Ret return type
         * @tparam Args Types of parameters
         * @param name Name of the static method
         * @param ptr Pointer to the static function
         */
        template <typename Ret, typename... Args>
        DocProxy static_method(const std::string &name, Ret (*ptr)(Args...));

        // ========================================================================
        // METHODS FROM BASE CLASSES
        // ========================================================================

        // CHANGED: Return vector of infos to support overloads
        std::vector<size_t> get_method_arities(const std::string &name) const;
        std::vector<std::vector<std::type_index>>
                                     get_method_arg_types_all(const std::string &name) const;
        std::vector<std::type_index> get_method_return_types(const std::string &name) const;

        // Keep old API for backward compatibility (returns first overload)
        size_t                              get_method_arity(const std::string &name) const;
        const std::vector<std::type_index> &get_method_arg_types(const std::string &name) const;
        std::type_index                     get_method_return_type(const std::string &name) const;

        /**
         * @brief Register non const method from base class
         */
        template <typename Base, typename Ret, typename... Args>
        DocProxy base_method(const std::string &name, Ret (Base::*ptr)(Args...));

        /**
         * @brief Register const method from base class
         */
        template <typename Base, typename Ret, typename... Args>
        DocProxy base_method(const std::string &name, Ret (Base::*ptr)(Args...) const);

        // ========================================================================
        // Register virtual methods
        // ========================================================================

        /**
         * @brief Register non-const virtual method
         */
        template <typename Ret, typename... Args>
        DocProxy virtual_method(const std::string &name, Ret (Class::*ptr)(Args...));

        /**
         * @brief register const virtual method
         */
        template <typename Ret, typename... Args>
        DocProxy virtual_method(const std::string &name, Ret (Class::*ptr)(Args...) const);

        /**
         * @brief Register pure virtual method - template-only version (non-const)
         * Use when you don't have/need the function pointer
         */
        template <typename Ret, typename... Args>
        DocProxy pure_virtual_method(const std::string &name);

        /**
         * @brief Register pure virtual method - template-only version (const)
         * Use when you don't have/need the function pointer
         */
        template <typename Ret, typename... Args>
        DocProxy pure_virtual_method_const(const std::string &name);

        /**
         * @brief Register non-const method that overrides a base virtual method
         */
        template <typename Ret, typename... Args>
        DocProxy override_method(const std::string &name, Ret (Class::*ptr)(Args...));

        /**
         * @brief Register const method that overrides a base virtual method
         */
        template <typename Ret, typename... Args>
        DocProxy override_method(const std::string &name, Ret (Class::*ptr)(Args...) const);

        // ========================================================================
        // Auto-detect properties from get/set method pairs
        // ========================================================================

        /**
         * @brief Automatically detect and register properties from getter/setter pairs
         */
        ClassMetadata &auto_detect_properties();

    private:
        /**
         * @brief Helper to convert string to lowercase
         */
        static std::string to_lower(const std::string &str);

        /**
         * @brief Register a property from getter/setter method pair
         */
        void register_property_from_methods(const std::string &property_name,
                                            const std::string &getter_name,
                                            const std::string &setter_name,
                                            std::type_index    value_type);

        /**
         * @brief Register a read-only property from a getter method
         */
        void register_readonly_property_from_method(const std::string &property_name,
                                                    const std::string &getter_name,
                                                    std::type_index    value_type);

    public:
        // ========================================================================
        // ACCESSEURS
        // ========================================================================

        const std::vector<Constructor> &constructors() const;

        /**
         * @brief Get constructor information with parameter types
         */
        const std::vector<ConstructorInfo> &constructor_infos() const;

        /**
         * @brief Liste des noms de champs
         */
        const std::vector<std::string> &fields() const;

        /**
         * @brief Liste des noms de methodes
         */
        const std::vector<std::string> &methods() const;

        /**
         * @brief Check if a method is marked as overloaded
         */
        bool is_method_overloaded(const std::string &name) const;

        /**
         * @brief Check if a method is const
         */
        bool is_method_const(const std::string &name) const;

        /**
         * @brief Get all overloads information for a specific method
         * @param method_name Name of the method
         * @return Const reference to vector of MethodInfo for all overloads
         * @throws std::runtime_error if method not found
         */
        const std::vector<MethodInfo> &method_info(const std::string &method_name) const;

        /**
         * @brief Recuperation de la valeur d'un champs
         */
        Any get_field(Class &obj, const std::string &name) const;

        /**
         * @brief Modifie la valeur d'un champ
         */
        void set_field(Class &obj, const std::string &name, Any value) const;

        /**
         * @brief Invoke a method on an object (with automatic overload resolution)
         */
        Any invoke_method(Class &obj, const std::string &name, std::vector<Any> args = {}) const;

        /**
         * @brief Invoke a method on an object (const version with automatic overload resolution)
         */
        Any invoke_method(const Class &obj, const std::string &name,
                          std::vector<Any> args = {}) const;

        /**
         * @brief Invoke a method on a pointer to an object (with automatic overload resolution)
         * Performs dynamic_cast to ensure type safety
         */
        template <typename BasePtr>
        Any invoke_method(BasePtr *ptr, const std::string &name, std::vector<Any> args = {}) const;

        /**
         * @brief Invoke a method on a shared_ptr to an object (with automatic overload resolution)
         */
        template <typename BasePtr>
        Any invoke_method(const std::shared_ptr<BasePtr> &ptr, const std::string &name,
                          std::vector<Any> args = {}) const;

        /**
         * @brief Invoke a method on a unique_ptr to an object (with automatic overload resolution)
         */
        template <typename BasePtr>
        Any invoke_method(const std::unique_ptr<BasePtr> &ptr, const std::string &name,
                          std::vector<Any> args = {}) const;

        /**
         * @brief Invoke a method on a pointer to a const object (with automatic overload
         * resolution)
         */
        template <typename BasePtr>
        Any invoke_method(const BasePtr *ptr, const std::string &name,
                          std::vector<Any> args = {}) const;

        /**
         * @brief Invoke a static method (no object instance required)
         */
        Any invoke_static_method(const std::string &name, std::vector<Any> args = {}) const;

        /**
         * @brief Check if a method is static
         */
        bool is_static_method(const std::string &name) const;

        /**
         * @brief Check if the class is instantiable
         */
        bool is_instantiable() const;

        /**
         * @brief Get the type of a field
         */
        std::type_index get_field_type(const std::string &name) const;

        /// List of property names (virtual fields via getter/setter)
        const std::vector<std::string> &properties() const;

        /// Get property info by name
        const PropertyInfo &get_property_info(const std::string &name) const;

        /// Check if a name is a property
        bool is_property(const std::string &name) const;

        /// Get property value (virtual field via getter)
        Any get_property(Class &obj, const std::string &name) const;

        /// Set property value (virtual field via setter)
        void set_property(Class &obj, const std::string &name, Any value) const;

        /// Get property type (virtual field)
        std::type_index get_property_type(const std::string &name) const;

    private:
        // Calcule l'offset d'une classe de base
        template <typename Base> size_t calculate_base_offset() const;

        // Generate a signature of a method
        template <typename Ret, typename... Args> static std::string make_signature();

        // To invoke non-const methods with arguments
        template <typename Ret, typename... Args, size_t... Is>
        static Any invoke_with_args(Class &obj, Ret (Class::*ptr)(Args...), std::vector<Any> &args,
                                    std::index_sequence<Is...>);

        // Helper to extract argument with numeric conversion support
        template <typename T>
        static auto extract_arg(Any &any_val) -> std::remove_cv_t<std::remove_reference_t<T>>;

        // To invoke const methods with arguments
        template <typename Ret, typename... Args, size_t... Is>
        static Any invoke_const_with_args(const Class      &obj, Ret (Class::*ptr)(Args...) const,
                                          std::vector<Any> &args, std::index_sequence<Is...>);

        // To invoke static methods with arguments
        template <typename Ret, typename... Args, size_t... Is>
        static Any invoke_static_with_args(Ret (*ptr)(Args...), std::vector<Any> &args,
                                           std::index_sequence<Is...>);
    };

} // namespace rosetta::core

#include "inline/class_metadata.hxx"