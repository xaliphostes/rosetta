// ============================================================================
// rosetta/core/class_metadata.hpp
//
// Class metadata for Rosetta introspection system.
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

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#endif

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
        explicit ClassMetadata(std::string name) : name_(std::move(name)) {
            inheritance_.is_abstract            = std::is_abstract_v<Class>;
            inheritance_.is_polymorphic         = std::is_polymorphic_v<Class>;
            inheritance_.has_virtual_destructor = std::has_virtual_destructor_v<Class>;
        }

        /**
         * @brief Generic dumper for any registered class
         */
        void dump(std::ostream &os) const {
            os << "\n=== Rosetta metadata for class: " << name() << " ===\n";

            // Instantiability
            os << "Instantiable: " << (is_instantiable() ? "true" : "false") << "\n";

            // Constructors (infer arity by calling the thunk with k args and
            // distinguishing the size-mismatch error from type mismatch)
            if constexpr (requires { this->constructors(); }) {
                const auto &ctors = constructors();
                os << "Constructors (" << ctors.size() << "):\n";

                auto detect_arity = [](const auto &ctor) -> int {
                    for (int k = 0; k <= 8; ++k) {
                        std::vector<Any> args;
                        args.reserve(k);
                        for (int i = 0; i < k; ++i)
                            args.emplace_back(0); // cheap placeholder

                        try {
                            (void)ctor(args); // success => arity is k
                            return k;
                        } catch (const std::runtime_error &e) {
                            // Your ctor thunk throws this exact message when the count doesn't
                            // match
                            if (std::string(e.what()).find("Constructor argument count mismatch") !=
                                std::string::npos) {
                                continue; // try next k
                            }
                            return k; // type mismatch => count matched, arity is k
                        } catch (...) {
                            return k; // conservative: count matched
                        }
                    }
                    return -1;
                };

                int idx = 0;
                for (const auto &ctor : ctors) {
                    int arity = detect_arity(ctor);
                    if (arity >= 0)
                        os << "  - [" << idx++ << "] (" << arity << " param"
                           << (arity == 1 ? "" : "s") << ")\n";
                    else
                        os << "  - [" << idx++ << "] (unknown arity)\n";
                }
            }

            // Fields
            const auto &f = fields();
            os << "Fields (" << f.size() << "):\n";
            for (const auto &name : f) {
                std::type_index ti        = get_field_type(name);
                std::string     type_name = get_readable_type_name(ti);
                os << "  - " << name << " : " << type_name << "\n";
            }

            // Methods - now with argument types and count (demangled)
            const auto &m = methods();
            os << "Methods (" << m.size() << "):\n";
            for (const auto &name : m) {
                auto it = method_info_.find(name);
                if (it != method_info_.end()) {
                    const MethodInfo &info = it->second;

                    // Display return type (demangled)
                    std::string return_type = get_readable_type_name(info.return_type);
                    os << "  - " << return_type << " " << name << "(";

                    // Display argument types (demangled)
                    for (size_t i = 0; i < info.arg_types.size(); ++i) {
                        std::string arg_type = get_readable_type_name(info.arg_types[i]);
                        os << arg_type;
                        if (i < info.arg_types.size() - 1) {
                            os << ", ";
                        }
                    }
                    os << ")";

                    // Display arity
                    os << " [" << info.arity << " arg" << (info.arity == 1 ? "" : "s") << "]\n";
                } else {
                    // Fallback for methods without stored info
                    os << "  - " << name << " (no type info available)\n";
                }
            }

            // Inheritance
            const auto &inh = inheritance();
            os << "Inheritance flags:\n";
            os << "  is_abstract            = " << (inh.is_abstract ? "true" : "false") << "\n";
            os << "  is_polymorphic         = " << (inh.is_polymorphic ? "true" : "false") << "\n";
            os << "  has_virtual_destructor = " << (inh.has_virtual_destructor ? "true" : "false")
               << "\n";
            os << "  base_count             = " << inh.total_base_count() << "\n";

            os << "===============================================\n";
        }

        /**
         * @brief Obtient le nom de la classe
         */
        const std::string &name() const { return name_; }

        /**
         * @brief Get inheritance information (const version)
         */
        const InheritanceInfo &inheritance() const { return inheritance_; }

        /**
         * @brief Get inheritance information (non-const version)
         */
        InheritanceInfo &inheritance() { return inheritance_; }

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
                                void (Class::*setter)(const T &)) {
            field_names_.push_back(name);

            // Store type information
            field_types_.emplace(name, std::type_index(typeid(T)));

            // Getter: invoke the getter method and wrap result in Any
            field_getters_[name] = [getter](Class &obj) -> Any {
                const Class &const_obj = obj;
                return Any((const_obj.*getter)());
            };

            // Setter: extract value from Any and invoke the setter method
            field_setters_[name] = [setter](Class &obj, Any value) {
                (obj.*setter)(value.as<T>());
            };

            return *this;
        }

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
                                void (Class::*setter)(const T &)) {
            field_names_.push_back(name);

            // Store type information
            field_types_.emplace(name, std::type_index(typeid(T)));

            // Getter: invoke the getter method and wrap result in Any
            field_getters_[name] = [getter](Class &obj) -> Any {
                const Class &const_obj = obj;
                return Any((const_obj.*getter)());
            };

            // Setter: extract value from Any and invoke the setter method
            field_setters_[name] = [setter](Class &obj, Any value) {
                (obj.*setter)(value.as<T>());
            };

            return *this;
        }

        /**
         * @brief Register a virtual field - variant with non-const reference getter
         *
         * This overload handles getters that return a non-const reference.
         */
        template <typename T>
        ClassMetadata &property(const std::string &name, T &(Class::*getter)(),
                                void (Class::*setter)(const T &)) {
            field_names_.push_back(name);

            // Store type information
            field_types_.emplace(name, std::type_index(typeid(T)));

            // Getter: invoke the getter method and wrap result in Any
            field_getters_[name] = [getter](Class &obj) -> Any { return Any((obj.*getter)()); };

            // Setter: extract value from Any and invoke the setter method
            field_setters_[name] = [setter](Class &obj, Any value) {
                (obj.*setter)(value.as<T>());
            };

            return *this;
        }

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
                                         const T &(Class::*getter)() const) {
            field_names_.push_back(name);

            // Store type information
            field_types_.emplace(name, std::type_index(typeid(T)));

            // Getter: invoke the getter method and wrap result in Any
            field_getters_[name] = [getter](Class &obj) -> Any {
                const Class &const_obj = obj;
                return Any((const_obj.*getter)());
            };

            // No setter - will throw if someone tries to set this field
            field_setters_[name] = [name](Class &, Any) {
                throw std::runtime_error("Cannot set read-only property: " + name);
            };

            return *this;
        }

        /**
         * @brief Register a read-only virtual field - variant returning by value
         *
         * This is the most common case for computed properties with primitive types.
         */
        template <typename T>
        ClassMetadata &readonly_property(const std::string &name, T (Class::*getter)() const) {
            field_names_.push_back(name);

            // Store type information
            field_types_.emplace(name, std::type_index(typeid(T)));

            // Getter: invoke the getter method and wrap result in Any
            field_getters_[name] = [getter](Class &obj) -> Any {
                const Class &const_obj = obj;
                return Any((const_obj.*getter)());
            };

            // No setter - will throw if someone tries to set this field
            field_setters_[name] = [name](Class &, Any) {
                throw std::runtime_error("Cannot set read-only property: " + name);
            };

            return *this;
        }

        /**
         * @brief Register a write-only virtual field (setter only, no getter)
         *
         * Useful for write-only properties or when the getter isn't const.
         */
        template <typename T>
        ClassMetadata &writeonly_property(const std::string &name,
                                          void (Class::*setter)(const T &)) {
            field_names_.push_back(name);

            // Store type information
            field_types_.emplace(name, std::type_index(typeid(T)));

            // No getter - will throw if someone tries to get this field
            field_getters_[name] = [name](Class &) -> Any {
                throw std::runtime_error("Cannot get write-only property: " + name);
            };

            // Setter: extract value from Any and invoke the setter method
            field_setters_[name] = [setter](Class &obj, Any value) {
                (obj.*setter)(value.as<T>());
            };

            return *this;
        }

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
                                     AccessSpecifier    access    = AccessSpecifier::Public) {
            static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

            std::string name   = base_name.empty() ? typeid(Base).name() : base_name;
            size_t      offset = calculate_base_offset<Base>();

            inheritance_.add_base(name, &typeid(Base), InheritanceType::Normal, access, offset);

            return *this;
        }

        /**
         * @brief Tells that this class virtually inherits from a base class
         * @tparam Base Type of the base class
         * @param base_name Name of the base (optional)
         * @param access Access specifier
         */
        template <typename Base>
        ClassMetadata &virtually_inherits_from(const std::string &base_name = "",
                                               AccessSpecifier access = AccessSpecifier::Public) {
            static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

            std::string name   = base_name.empty() ? typeid(Base).name() : base_name;
            size_t      offset = 0;

            inheritance_.add_base(name, &typeid(Base), InheritanceType::Virtual, access, offset);

            return *this;
        }

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
        template <typename... Args> ClassMetadata &constructor() {
            constructors_.emplace_back([](const std::vector<Any> &args) -> Any {
                if (args.size() != sizeof...(Args))
                    throw std::runtime_error("Constructor argument count mismatch");

                // Use index_sequence to properly unpack arguments
                return construct_with_indices<Args...>(args, std::index_sequence_for<Args...>{});
            });
            return *this;
        }

    private:
        // Helper to construct with proper indexing
        template <typename... Args, std::size_t... Is>
        static Any construct_with_indices(const std::vector<Any> &args, std::index_sequence<Is...>) {
            return Any(Class(args[Is].as<std::remove_cv_t<std::remove_reference_t<Args>>>()...));
        }

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
        template <typename T> ClassMetadata &field(const std::string &name, T Class::*ptr) {
            field_names_.push_back(name);

            // Store type information using emplace to avoid default construction issues
            field_types_.emplace(name, std::type_index(typeid(T)));

            field_getters_[name] = [ptr](Class &obj) -> Any { return Any(obj.*ptr); };

            field_setters_[name] = [ptr](Class &obj, Any value) { obj.*ptr = value.as<T>(); };

            return *this;
        }

        /**
         * @brief Enregistre un champ d'une classe de base
         * @tparam Base Type de la classe de base
         * @tparam T Type du champ
         * @param name Nom du champ
         * @param ptr Pointeur vers membre de la base
         */
        template <typename Base, typename T>
        ClassMetadata &base_field(const std::string &name, T Base::*ptr) {
            static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

            field_names_.push_back(name);

            // Store type information using emplace to avoid default construction issues
            field_types_.emplace(name, std::type_index(typeid(T)));

            field_getters_[name] = [ptr](Class &obj) -> Any {
                Base &base = static_cast<Base &>(obj);
                return Any(base.*ptr);
            };

            field_setters_[name] = [ptr](Class &obj, Any value) {
                Base &base = static_cast<Base &>(obj);
                base.*ptr  = value.as<T>();
            };

            return *this;
        }

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
        ClassMetadata &method(const std::string &name, Ret (Class::*ptr)(Args...)) {
            method_names_.push_back(name);

            // Create and store method info
            MethodInfo info;
            info.arity       = sizeof...(Args);
            info.return_type = std::type_index(typeid(Ret));
            info.arg_types   = {std::type_index(typeid(Args))...}; // Pack expansion

            info.invoker = [ptr](Class &obj, std::vector<Any> args) -> Any {
                if constexpr (sizeof...(Args) == 0) {
                    if constexpr (std::is_void_v<Ret>) {
                        (obj.*ptr)();
                        return Any(0);
                    } else {
                        return Any((obj.*ptr)());
                    }
                } else {
                    return invoke_with_args(obj, ptr, args, std::index_sequence_for<Args...>{});
                }
            };

            method_info_[name] = info;
            methods_[name]     = info.invoker;

            return *this;
        }

        // ========================================================================
        // Register const methods
        // ========================================================================

        /**
         * @brief Register const method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &method(const std::string &name, Ret (Class::*ptr)(Args...) const) {
            method_names_.push_back(name);

            // Create and store method info
            MethodInfo info;
            info.arity       = sizeof...(Args);
            info.return_type = std::type_index(typeid(Ret));
            info.arg_types   = {std::type_index(typeid(Args))...}; // Pack expansion

            const_methods_[name] = [ptr](const Class &obj, std::vector<Any> args) -> Any {
                if constexpr (sizeof...(Args) == 0) {
                    if constexpr (std::is_void_v<Ret>) {
                        (obj.*ptr)();
                        return Any(0);
                    } else {
                        return Any((obj.*ptr)());
                    }
                } else {
                    return invoke_const_with_args(obj, ptr, args,
                                                  std::index_sequence_for<Args...>{});
                }
            };

            // Wrapper pour appel depuis objet non-const
            info.invoker = [ptr](Class &obj, std::vector<Any> args) -> Any {
                const Class &const_obj = obj;
                if constexpr (sizeof...(Args) == 0) {
                    if constexpr (std::is_void_v<Ret>) {
                        (const_obj.*ptr)();
                        return Any(0);
                    } else {
                        return Any((const_obj.*ptr)());
                    }
                } else {
                    return invoke_const_with_args(const_obj, ptr, args,
                                                  std::index_sequence_for<Args...>{});
                }
            };

            method_info_[name] = info;
            methods_[name]     = info.invoker;

            return *this;
        }

        // ========================================================================
        // METHODS FROM BASE CLASSES
        // ========================================================================

        size_t get_method_arity(const std::string &name) const {
            return method_info_.at(name).arity;
        }

        const std::vector<std::type_index> &get_method_arg_types(const std::string &name) const {
            return method_info_.at(name).arg_types;
        }

        std::type_index get_method_return_type(const std::string &name) const {
            return method_info_.at(name).return_type;
        }

        /**
         * @brief Register non const method from base class
         */
        template <typename Base, typename Ret, typename... Args>
        ClassMetadata &base_method(const std::string &name, Ret (Base::*ptr)(Args...)) {
            static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

            method_names_.push_back(name);

            MethodInfo info;
            info.arity       = sizeof...(Args);
            info.return_type = std::type_index(typeid(Ret));
            info.arg_types   = {std::type_index(typeid(Args))...}; // Pack expansion

            info.invoker = [ptr](Class &obj, std::vector<Any> args) -> Any {
                Base &base = static_cast<Base &>(obj);
                if constexpr (sizeof...(Args) == 0) {
                    if constexpr (std::is_void_v<Ret>) {
                        (base.*ptr)();
                        return Any(0);
                    } else {
                        return Any((base.*ptr)());
                    }
                } else {
                    return invoke_with_args(base, ptr, args, std::index_sequence_for<Args...>{});
                }
            };

            method_info_[name] = std::move(info);
            methods_[name]     = info.invoker; // Keep backward compatibility

            return *this;
        }

        /**
         * @brief Register const method from base class
         */
        template <typename Base, typename Ret, typename... Args>
        ClassMetadata &base_method(const std::string &name, Ret (Base::*ptr)(Args...) const) {
            static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

            method_names_.push_back(name);

            MethodInfo info;
            info.arity       = sizeof...(Args);
            info.return_type = std::type_index(typeid(Ret));
            info.arg_types   = {std::type_index(typeid(Args))...}; // Pack expansion

            info.invoker = [ptr](const Class &obj, std::vector<Any> args) -> Any {
                const Base &base = static_cast<const Base &>(obj);
                if constexpr (sizeof...(Args) == 0) {
                    if constexpr (std::is_void_v<Ret>) {
                        (base.*ptr)();
                        return Any(0);
                    } else {
                        return Any((base.*ptr)());
                    }
                } else {
                    return invoke_const_with_args(base, ptr, args,
                                                  std::index_sequence_for<Args...>{});
                }
            };

            method_info_[name]   = std::move(info);
            methods_[name]       = info.invoker; // Keep backward compatibility
            const_methods_[name] = info.invoker; // Keep backward compatibility

            return *this;
        }

        // ========================================================================
        // Register virtual methods
        // ========================================================================

        /**
         * @brief Register non-const virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &virtual_method(const std::string &name, Ret (Class::*ptr)(Args...)) {
            std::string signature = make_signature<Ret, Args...>();
            VirtualMethodRegistry::instance().register_virtual_method<Class>(name, signature,
                                                                             false);
            inheritance_.vtable.add_virtual_method(name, signature, false);
            return method(name, ptr);
        }

        /**
         * @brief register const virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &virtual_method(const std::string &name, Ret (Class::*ptr)(Args...) const) {
            std::string signature = make_signature<Ret, Args...>();
            VirtualMethodRegistry::instance().register_virtual_method<Class>(name, signature,
                                                                             false);
            inheritance_.vtable.add_virtual_method(name, signature, false);
            return method(name, ptr);
        }

        /**
         * @brief Register pure virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &pure_virtual_method(const std::string &name,
                                           Ret (Class::*ptr)(Args...) = nullptr) {
            std::string signature = make_signature<Ret, Args...>();
            VirtualMethodRegistry::instance().register_virtual_method<Class>(name, signature, true);
            inheritance_.vtable.add_virtual_method(name, signature, true);
            method_names_.push_back(name);
            return *this;
        }

        /**
         * @brief Register non-const method that overrides a base virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &override_method(const std::string &name, Ret (Class::*ptr)(Args...)) {
            auto *vmethod = inheritance_.vtable.find_method(name);
            if (vmethod) {
                const_cast<VirtualMethodInfo *>(vmethod)->is_override = true;
            }
            return virtual_method(name, ptr);
        }

        /**
         * @brief Register const method that overrides a base virtual method
         */
        template <typename Ret, typename... Args>
        ClassMetadata &override_method(const std::string &name, Ret (Class::*ptr)(Args...) const) {
            auto *vmethod = inheritance_.vtable.find_method(name);
            if (vmethod) {
                const_cast<VirtualMethodInfo *>(vmethod)->is_override = true;
            }
            return virtual_method(name, ptr);
        }

        // ========================================================================
        // ACCESSEURS
        // ========================================================================

        const std::vector<Constructor> &constructors() const { return constructors_; }

        /**
         * @brief Liste des noms de champs
         */
        const std::vector<std::string> &fields() const { return field_names_; }

        /**
         * @brief Liste des noms de methodes
         */
        const std::vector<std::string> &methods() const { return method_names_; }

        /**
         * @brief Recuperation de la valeur d'un champs
         */
        Any get_field(Class &obj, const std::string &name) const {
            return field_getters_.at(name)(obj);
        }

        /**
         * @brief Modifie la valeur d'un champ
         */
        void set_field(Class &obj, const std::string &name, Any value) const {
            field_setters_.at(name)(obj, std::move(value));
        }

        /**
         * @brief Invoke a method on an object
         */
        Any invoke_method(Class &obj, const std::string &name, std::vector<Any> args = {}) const {
            return methods_.at(name)(obj, std::move(args));
        }

        /**
         * @brief Invoke a method on an object (const version)
         */
        Any invoke_method(const Class &obj, const std::string &name,
                          std::vector<Any> args = {}) const {
            return const_methods_.at(name)(obj, std::move(args));
        }

        /**
         * @brief Check if the class is instantiable
         */
        bool is_instantiable() const {
            return !inheritance_.is_abstract &&
                   !VirtualMethodRegistry::instance().has_pure_virtual_methods<Class>();
        }

        /**
         * @brief Get the type of a field
         * @param name Nom du champ
         * @return type_index of the field, or typeid(void) if not found
         */
        std::type_index get_field_type(const std::string &name) const {
            auto it = field_types_.find(name);
            if (it != field_types_.end()) {
                return it->second;
            }
            return std::type_index(typeid(void));
        }

    private:
        // Calcule l'offset d'une classe de base
        template <typename Base> size_t calculate_base_offset() const {
            if constexpr (std::is_base_of_v<Base, Class> && !std::is_abstract_v<Class>) {
                alignas(Class) char buffer[sizeof(Class)];
                Class              *derived_ptr = reinterpret_cast<Class *>(buffer);
                Base               *base_ptr    = static_cast<Base *>(derived_ptr);
                return reinterpret_cast<char *>(base_ptr) - reinterpret_cast<char *>(derived_ptr);
            }
            return 0;
        }

        // Generate a signature of a method
        template <typename Ret, typename... Args> static std::string make_signature() {
            std::string sig = typeid(Ret).name();
            sig += "(";
            ((sig += typeid(Args).name(), sig += ","), ...);
            if (sizeof...(Args) > 0)
                sig.pop_back();
            sig += ")";
            return sig;
        }

        // To invoke non-const methods with arguments
        template <typename Ret, typename... Args, size_t... Is>
        static Any invoke_with_args(Class &obj, Ret (Class::*ptr)(Args...), std::vector<Any> &args,
                                    std::index_sequence<Is...>) {
            try {
                if constexpr (std::is_void_v<Ret>) {
                    (obj.*ptr)(extract_arg<Args>(args[Is])...);
                    return Any(0);
                } else {
                    return Any((obj.*ptr)(extract_arg<Args>(args[Is])...));
                }
            } catch (const std::bad_cast &e) {
                std::string error = "Type mismatch in method arguments. Expected types: ";
                ((error += typeid(Args).name(), error += " "), ...);
                throw std::runtime_error(error);
            }
        }

        // Helper to extract argument with numeric conversion support
        template <typename T>
        static auto extract_arg(Any &any_val) -> std::remove_cv_t<std::remove_reference_t<T>> {
            using RawType = std::remove_cv_t<std::remove_reference_t<T>>;

            std::type_index actual_type   = any_val.get_type_index();
            std::type_index expected_type = std::type_index(typeid(RawType));

            // Direct match
            if (actual_type == expected_type) {
                return any_val.as<RawType>();
            }

            // Numeric conversions
            if constexpr (std::is_arithmetic_v<RawType>) {
                if (actual_type == std::type_index(typeid(double))) {
                    return static_cast<RawType>(any_val.as<double>());
                }
                if (actual_type == std::type_index(typeid(int))) {
                    return static_cast<RawType>(any_val.as<int>());
                }
                if (actual_type == std::type_index(typeid(float))) {
                    return static_cast<RawType>(any_val.as<float>());
                }
            }

            // Fallback to normal extraction
            return any_val.as<RawType>();
        }

        // To invoke const methods with arguments
        template <typename Ret, typename... Args, size_t... Is>
        static Any invoke_const_with_args(const Class      &obj, Ret (Class::*ptr)(Args...) const,
                                          std::vector<Any> &args, std::index_sequence<Is...>) {
            try {
                if constexpr (std::is_void_v<Ret>) {
                    (obj.*ptr)(extract_arg<Args>(args[Is])...);
                    return Any(0);
                } else {
                    return Any((obj.*ptr)(extract_arg<Args>(args[Is])...));
                }
            } catch (const std::bad_cast &e) {
                std::string error = "Type mismatch in method arguments. Expected types: ";
                ((error += typeid(Args).name(), error += " "), ...);
                throw std::runtime_error(error);
            }
        }
    };

} // namespace rosetta::core