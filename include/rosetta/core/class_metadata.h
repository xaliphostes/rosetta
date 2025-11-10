// ============================================================================
// rosetta/core/class_metadata.hpp
//
// Class metadata for Rosetta introspection system.
// ============================================================================
#pragma once
#include "any.h"
#include "demangler.h"
#include "inheritance_info.h"
// #include "function_auto_registration.h"
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

        std::unordered_map<std::string, std::function<Any(Class &)>>       field_getters_;
        std::unordered_map<std::string, std::function<void(Class &, Any)>> field_setters_;
        std::unordered_map<std::string, std::function<Any(Class &, std::vector<Any>)>> methods_;
        std::unordered_map<std::string, std::function<Any(const Class &, std::vector<Any>)>>
            const_methods_;

        // ----------------------------------------

        using Constructor = std::function<Any(std::vector<Any>)>;
        std::vector<Constructor> constructors_;

        struct ConstructorInfo {
            std::function<Any(std::vector<Any>)> invoker;
            std::vector<std::type_index>         param_types;
            size_t                               arity = 0;
        };
        std::vector<ConstructorInfo> constructor_infos_;

        // ----------------------------------------

        std::vector<std::string> field_names_;
        // Store type information for fields (for JS binding)
        std::unordered_map<std::string, std::type_index> field_types_;

        // ----------------------------------------

        std::vector<std::string> method_names_;

        struct MethodInfo {
            std::function<Any(Class &, std::vector<Any>)> invoker;
            std::vector<std::type_index>                  arg_types;
            std::type_index return_type = std::type_index(typeid(void));
            size_t          arity       = 0;
        };
        std::unordered_map<std::string, MethodInfo> method_info_;

        // ----------------------------------------

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
         *
         * This allows registering a "field" even when the underlying member is private,
         * as long as you have public getter/setter methods.
         *
         * Example:
         * ```cpp
         * class Model {
         *     std::vector<Surface> surfaces_;  // private!
         * public:
         *     const std::vector<Surface>& getSurfaces() const { return surfaces_; }
         *     void setSurfaces(const std::vector<Surface>& s) { surfaces_ = s; }
         * };
         *
         * ROSETTA_REGISTER_CLASS(Model)
         *     .property<std::vector<Surface>>("surfaces",
         *                                     &Model::getSurfaces,
         *                                     &Model::setSurfaces);
         * ```
         */
        template <typename T>
        ClassMetadata &property(const std::string &name, const T &(Class::*getter)() const,
                                void (Class::*setter)(const T &));

        /**
         * @brief Register a virtual field - variant with getter returning by value
         *
         * This overload handles getters that return by value instead of const reference.
         *
         * Example:
         * ```cpp
         * class Rectangle {
         *     double width_;
         * public:
         *     double getWidth() const { return width_; }  // Returns by value
         *     void setWidth(double w) { width_ = w; }
         * };
         *
         * ROSETTA_REGISTER_CLASS(Rectangle)
         *     .property<double>("width", &Rectangle::getWidth, &Rectangle::setWidth);
         * ```
         */
        template <typename T>
        ClassMetadata &property(const std::string &name, T (Class::*getter)() const,
                                void (Class::*setter)(const T &));

        /**
         * @brief Register a virtual field - variant with non-const reference getter
         *
         * This overload handles getters that return a non-const reference.
         */
        template <typename T>
        ClassMetadata &property(const std::string &name, T &(Class::*getter)(),
                                void (Class::*setter)(const T &));

        /**
         * @brief Register a read-only virtual field (getter only, no setter)
         *
         * For getters returning by const reference.
         *
         * Example:
         * ```cpp
         * class Rectangle {
         *     double width_, height_;
         * public:
         *     double getArea() const { return width_ * height_; }  // Computed, no setter
         * };
         *
         * ROSETTA_REGISTER_CLASS(Rectangle)
         *     .readonly_property<double>("area", &Rectangle::getArea);
         * ```
         */
        template <typename T>
        ClassMetadata &readonly_property(const std::string &name,
                                         const T &(Class::*getter)() const);

        /**
         * @brief Register a read-only virtual field - variant returning by value
         *
         * This is the most common case for computed properties with primitive types.
         */
        template <typename T>
        ClassMetadata &readonly_property(const std::string &name, T (Class::*getter)() const);

        /**
         * @brief Register a write-only virtual field (setter only, no getter)
         *
         * Useful for write-only properties or when the getter isn't const.
         */
        template <typename T>
        ClassMetadata &writeonly_property(const std::string &name,
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
         * @example
         * ```cpp
         * ROSETTA_REGISTER_CLASS(Vector3D)
         *   .constructor<>()
         *   .constructor<double, double, double>();
         * ```
         *
         * @example
         * ```cpp
         * auto &meta = ROSETTA_GET_META(Vector3D);
         *
         * Vector3D v1 = meta.construct().as<Vector3D>();                    // default
         * Vector3D v2 = meta.construct(3.0, 4.0, 5.0).as<Vector3D>();       // parametric
         * ```
         */
        template <typename... Args> ClassMetadata &constructor();

    private:
        // Helper to construct with proper indexing
        template <typename... Args, std::size_t... Is>
        static Any construct_with_indices(const std::vector<Any> &args, std::index_sequence<Is...>);

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
        template <typename T> ClassMetadata &field(const std::string &name, T Class::*ptr);

        /**
         * @brief Enregistre un champ d'une classe de base
         * @tparam Base Type de la classe de base
         * @tparam T Type du champ
         * @param name Nom du champ
         * @param ptr Pointeur vers membre de la base
         */
        template <typename Base, typename T>
        ClassMetadata &base_field(const std::string &name, T Base::*ptr);

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
        ClassMetadata &method(const std::string &name, Ret (Class::*ptr)(Args...));

        // ========================================================================
        // Register const methods
        // ========================================================================

        /**
         * @brief Register const method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &method(const std::string &name, Ret (Class::*ptr)(Args...) const);

        // ========================================================================
        // METHODS FROM BASE CLASSES
        // ========================================================================

        size_t get_method_arity(const std::string &name) const;

        const std::vector<std::type_index> &get_method_arg_types(const std::string &name) const;

        std::type_index get_method_return_type(const std::string &name) const;

        /**
         * @brief Register non const method from base class
         */
        template <typename Base, typename Ret, typename... Args>
        ClassMetadata &base_method(const std::string &name, Ret (Base::*ptr)(Args...));

        /**
         * @brief Register const method from base class
         */
        template <typename Base, typename Ret, typename... Args>
        ClassMetadata &base_method(const std::string &name, Ret (Base::*ptr)(Args...) const);

        // ========================================================================
        // Register virtual methods
        // ========================================================================

        /**
         * @brief Register non-const virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &virtual_method(const std::string &name, Ret (Class::*ptr)(Args...));

        /**
         * @brief register const virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &virtual_method(const std::string &name, Ret (Class::*ptr)(Args...) const);

        /**
         * @brief Register pure virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &pure_virtual_method(const std::string &name,
                                           Ret (Class::*ptr)(Args...) = nullptr);

        /**
         * @brief Register non-const method that overrides a base virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &override_method(const std::string &name, Ret (Class::*ptr)(Args...));

        /**
         * @brief Register const method that overrides a base virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &override_method(const std::string &name, Ret (Class::*ptr)(Args...) const);

        // ========================================================================
        // Auto-detect properties from get/set method pairs
        // ========================================================================

        /**
         * @brief Automatically detect and register properties from getter/setter pairs
         *
         * Scans all registered methods for patterns like:
         * - getXyz() / setXyz(value) → property "xyz"
         * - GetXyz() / SetXyz(value) → property "xyz"
         *
         * The getter must have 0 parameters and a non-void return type.
         * The setter must have 1 parameter and match the getter's return type.
         *
         * @return Reference to this for chaining
         *
         * @example
         * ```cpp
         * ROSETTA_REGISTER_CLASS(Model)
         *     .method("getSurfaces", &Model::getSurfaces)
         *     .method("setSurfaces", &Model::setSurfaces)
         *     .auto_detect_properties();  // Creates "surfaces" property
         * ```
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
         * @brief Recuperation de la valeur d'un champs
         */
        Any get_field(Class &obj, const std::string &name) const;

        /**
         * @brief Modifie la valeur d'un champ
         */
        void set_field(Class &obj, const std::string &name, Any value) const;

        /**
         * @brief Invoke a method on an object
         */
        Any invoke_method(Class &obj, const std::string &name, std::vector<Any> args = {}) const;

        /**
         * @brief Invoke a method on an object (const version)
         */
        Any invoke_method(const Class &obj, const std::string &name,
                          std::vector<Any> args = {}) const;

        /**
         * @brief Check if the class is instantiable
         */
        bool is_instantiable() const;

        /**
         * @brief Get the type of a field
         * @param name Nom du champ
         * @return type_index of the field, or typeid(void) if not found
         */
        std::type_index get_field_type(const std::string &name) const;

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
    };

} // namespace rosetta::core

#include "inline/class_metadata.hxx"
