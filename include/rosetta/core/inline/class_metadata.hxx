#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#endif

namespace rosetta::core {

    // ============================================================================
    // ClassMetadata
    // ============================================================================

    template <typename Class>
    inline ClassMetadata<Class>::ClassMetadata(std::string name) : name_(std::move(name)) {
        inheritance_.is_abstract            = std::is_abstract_v<Class>;
        inheritance_.is_polymorphic         = std::is_polymorphic_v<Class>;
        inheritance_.has_virtual_destructor = std::has_virtual_destructor_v<Class>;
    }

    template <typename Class> inline void ClassMetadata<Class>::dump(std::ostream &os) const {
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
                    os << "  - [" << idx++ << "] (" << arity << " param" << (arity == 1 ? "" : "s")
                       << ")\n";
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

    template <typename Class> inline const std::string &ClassMetadata<Class>::name() const {
        return name_;
    }

    template <typename Class>
    inline const InheritanceInfo &ClassMetadata<Class>::inheritance() const {
        return inheritance_;
    }

    template <typename Class> inline InheritanceInfo &ClassMetadata<Class>::inheritance() {
        return inheritance_;
    }

    // ========================================================================
    // Virtual field registration (properties with getter/setter)
    // ========================================================================

    template <typename Class>
    template <typename T>
    inline ClassMetadata<Class> &ClassMetadata<Class>::property(const std::string &name,
                                                                const T &(Class::*getter)() const,
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
        field_setters_[name] = [setter](Class &obj, Any value) { (obj.*setter)(value.as<T>()); };

        return *this;
    }

    template <typename Class>
    template <typename T>
    inline ClassMetadata<Class> &ClassMetadata<Class>::property(const std::string &name,
                                                                T (Class::*getter)() const,
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
        field_setters_[name] = [setter](Class &obj, Any value) { (obj.*setter)(value.as<T>()); };

        return *this;
    }

    template <typename Class>
    template <typename T>
    inline ClassMetadata<Class> &ClassMetadata<Class>::property(const std::string &name,
                                                                T &(Class::*getter)(),
                                                                void (Class::*setter)(const T &)) {
        field_names_.push_back(name);

        // Store type information
        field_types_.emplace(name, std::type_index(typeid(T)));

        // Getter: invoke the getter method and wrap result in Any
        field_getters_[name] = [getter](Class &obj) -> Any { return Any((obj.*getter)()); };

        // Setter: extract value from Any and invoke the setter method
        field_setters_[name] = [setter](Class &obj, Any value) { (obj.*setter)(value.as<T>()); };

        return *this;
    }

    template <typename Class>
    template <typename T>
    inline ClassMetadata<Class> &ClassMetadata<Class>::readonly_property(const std::string &name,
                                                                         const T &(Class::*getter)()
                                                                             const) {
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

    template <typename Class>
    template <typename T>
    inline ClassMetadata<Class> &
    ClassMetadata<Class>::readonly_property(const std::string &name, T (Class::*getter)() const) {
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

    template <typename Class>
    template <typename T>
    inline ClassMetadata<Class> &
    ClassMetadata<Class>::writeonly_property(const std::string &name,
                                             void (Class::*setter)(const T &)) {
        field_names_.push_back(name);

        // Store type information
        field_types_.emplace(name, std::type_index(typeid(T)));

        // No getter - will throw if someone tries to get this field
        field_getters_[name] = [name](Class &) -> Any {
            throw std::runtime_error("Cannot get write-only property: " + name);
        };

        // Setter: extract value from Any and invoke the setter method
        field_setters_[name] = [setter](Class &obj, Any value) { (obj.*setter)(value.as<T>()); };

        return *this;
    }

    // ========================================================================
    // Declare inheritance
    // ========================================================================

    template <typename Class>
    template <typename Base>
    inline ClassMetadata<Class> &ClassMetadata<Class>::inherits_from(const std::string &base_name,
                                                                     AccessSpecifier    access) {
        static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

        std::string name   = base_name.empty() ? typeid(Base).name() : base_name;
        size_t      offset = calculate_base_offset<Base>();

        inheritance_.add_base(name, &typeid(Base), InheritanceType::Normal, access, offset);

        return *this;
    }

    template <typename Class>
    template <typename Base>
    inline ClassMetadata<Class> &
    ClassMetadata<Class>::virtually_inherits_from(const std::string &base_name,
                                                  AccessSpecifier    access) {
        static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

        std::string name   = base_name.empty() ? typeid(Base).name() : base_name;
        size_t      offset = 0;

        inheritance_.add_base(name, &typeid(Base), InheritanceType::Virtual, access, offset);

        return *this;
    }

    // ========================================================================
    // Constructor registration
    // ========================================================================

    template <typename Class>
    template <typename... Args>
    inline ClassMetadata<Class> &ClassMetadata<Class>::constructor() {
        // Store in old-style vector for backward compatibility
        constructors_.emplace_back([](const std::vector<Any> &args) -> Any {
            if (args.size() != sizeof...(Args))
                throw std::runtime_error("Constructor argument count mismatch");

            // Use index_sequence to properly unpack arguments
            return construct_with_indices<Args...>(args, std::index_sequence_for<Args...>{});
        });

        // Store in new ConstructorInfo with type information
        ConstructorInfo info;
        info.arity       = sizeof...(Args);
        info.param_types = {std::type_index(typeid(Args))...};
        info.invoker     = [](const std::vector<Any> &args) -> Any {
            if (args.size() != sizeof...(Args))
                throw std::runtime_error("Constructor argument count mismatch");
            return construct_with_indices<Args...>(args, std::index_sequence_for<Args...>{});
        };
        constructor_infos_.push_back(info);

        return *this;
    }

    // Helper to construct with proper indexing
    template <typename Class>
    template <typename... Args, std::size_t... Is>
    inline Any ClassMetadata<Class>::construct_with_indices(const std::vector<Any> &args,
                                                            std::index_sequence<Is...>) {
        return Any(Class(args[Is].as<std::remove_cv_t<std::remove_reference_t<Args>>>()...));
    }

    // ========================================================================
    // Field registration
    // ========================================================================

    template <typename Class>
    template <typename T>
    inline ClassMetadata<Class> &ClassMetadata<Class>::field(const std::string &name,
                                                             T Class::*ptr) {
        field_names_.push_back(name);

        // Store type information using emplace to avoid default construction issues
        field_types_.emplace(name, std::type_index(typeid(T)));

        field_getters_[name] = [ptr](Class &obj) -> Any { return Any(obj.*ptr); };

        field_setters_[name] = [ptr](Class &obj, Any value) { obj.*ptr = value.as<T>(); };

        return *this;
    }

    template <typename Class>
    template <typename Base, typename T>
    inline ClassMetadata<Class> &ClassMetadata<Class>::base_field(const std::string &name,
                                                                  T Base::*ptr) {
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

    template <typename Class>
    template <typename Ret, typename... Args>
    inline ClassMetadata<Class> &ClassMetadata<Class>::method(const std::string &name,
                                                              Ret (Class::*ptr)(Args...)) {
        // auto_register_function_converters(ptr);

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

    template <typename Class>
    template <typename Ret, typename... Args>
    inline ClassMetadata<Class> &ClassMetadata<Class>::method(const std::string &name,
                                                              Ret (Class::*ptr)(Args...) const) {
        // auto_register_function_converters(ptr);

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
                return invoke_const_with_args(obj, ptr, args, std::index_sequence_for<Args...>{});
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

    template <typename Class>
    inline size_t ClassMetadata<Class>::get_method_arity(const std::string &name) const {
        return method_info_.at(name).arity;
    }

    template <typename Class>
    inline const std::vector<std::type_index> &
    ClassMetadata<Class>::get_method_arg_types(const std::string &name) const {
        return method_info_.at(name).arg_types;
    }

    template <typename Class>
    inline std::type_index
    ClassMetadata<Class>::get_method_return_type(const std::string &name) const {
        return method_info_.at(name).return_type;
    }

    template <typename Class>
    template <typename Base, typename Ret, typename... Args>
    inline ClassMetadata<Class> &ClassMetadata<Class>::base_method(const std::string &name,
                                                                   Ret (Base::*ptr)(Args...)) {
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

    template <typename Class>
    template <typename Base, typename Ret, typename... Args>
    inline ClassMetadata<Class> &
    ClassMetadata<Class>::base_method(const std::string &name, Ret (Base::*ptr)(Args...) const) {
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
                return invoke_const_with_args(base, ptr, args, std::index_sequence_for<Args...>{});
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

    template <typename Class>
    template <typename Ret, typename... Args>
    inline ClassMetadata<Class> &ClassMetadata<Class>::virtual_method(const std::string &name,
                                                                      Ret (Class::*ptr)(Args...)) {
        std::string signature = make_signature<Ret, Args...>();
        VirtualMethodRegistry::instance().register_virtual_method<Class>(name, signature, false);
        inheritance_.vtable.add_virtual_method(name, signature, false);
        return method(name, ptr);
    }

    template <typename Class>
    template <typename Ret, typename... Args>
    inline ClassMetadata<Class> &ClassMetadata<Class>::virtual_method(const std::string &name,
                                                                      Ret (Class::*ptr)(Args...)
                                                                          const) {
        std::string signature = make_signature<Ret, Args...>();
        VirtualMethodRegistry::instance().register_virtual_method<Class>(name, signature, false);
        inheritance_.vtable.add_virtual_method(name, signature, false);
        return method(name, ptr);
    }

    template <typename Class>
    template <typename Ret, typename... Args>
    inline ClassMetadata<Class> &
    ClassMetadata<Class>::pure_virtual_method(const std::string &name, Ret (Class::*ptr)(Args...)) {
        std::string signature = make_signature<Ret, Args...>();
        VirtualMethodRegistry::instance().register_virtual_method<Class>(name, signature, true);
        inheritance_.vtable.add_virtual_method(name, signature, true);
        method_names_.push_back(name);
        return *this;
    }

    template <typename Class>
    template <typename Ret, typename... Args>
    inline ClassMetadata<Class> &ClassMetadata<Class>::override_method(const std::string &name,
                                                                       Ret (Class::*ptr)(Args...)) {
        auto *vmethod = inheritance_.vtable.find_method(name);
        if (vmethod) {
            const_cast<VirtualMethodInfo *>(vmethod)->is_override = true;
        }
        return virtual_method(name, ptr);
    }

    template <typename Class>
    template <typename Ret, typename... Args>
    inline ClassMetadata<Class> &ClassMetadata<Class>::override_method(const std::string &name,
                                                                       Ret (Class::*ptr)(Args...)
                                                                           const) {
        auto *vmethod = inheritance_.vtable.find_method(name);
        if (vmethod) {
            const_cast<VirtualMethodInfo *>(vmethod)->is_override = true;
        }
        return virtual_method(name, ptr);
    }

    // ========================================================================
    // Auto-detect properties from get/set method pairs
    // ========================================================================

    template <typename Class>
    inline ClassMetadata<Class> &ClassMetadata<Class>::auto_detect_properties() {
        // Build a map of potential getters: propertyName -> method info
        std::unordered_map<std::string, std::string> getters; // property_name -> method_name
        std::unordered_map<std::string, std::string> setters; // property_name -> method_name

        for (const auto &method_name : method_names_) {
            std::string lower_name = to_lower(method_name);

            // Check for getter pattern: get* or Get*
            if (method_name.size() > 3) {
                if ((method_name[0] == 'g' && method_name[1] == 'e' && method_name[2] == 't') ||
                    (method_name[0] == 'G' && method_name[1] == 'e' && method_name[2] == 't')) {

                    // Extract property name (everything after "get")
                    std::string property_name = method_name.substr(3);
                    if (!property_name.empty()) {
                        // Convert first char to lowercase for property name
                        property_name[0] = std::tolower(property_name[0]);

                        // Verify it's a valid getter (0 args, non-void return)
                        auto it = method_info_.find(method_name);
                        if (it != method_info_.end() && it->second.arity == 0 &&
                            it->second.return_type != std::type_index(typeid(void))) {
                            getters[property_name] = method_name;
                        }
                    }
                }
            }

            // Check for setter pattern: set* or Set*
            if (method_name.size() > 3) {
                if ((method_name[0] == 's' && method_name[1] == 'e' && method_name[2] == 't') ||
                    (method_name[0] == 'S' && method_name[1] == 'e' && method_name[2] == 't')) {

                    // Extract property name (everything after "set")
                    std::string property_name = method_name.substr(3);
                    if (!property_name.empty()) {
                        // Convert first char to lowercase for property name
                        property_name[0] = std::tolower(property_name[0]);

                        // Verify it's a valid setter (1 arg)
                        auto it = method_info_.find(method_name);
                        if (it != method_info_.end() && it->second.arity == 1) {
                            setters[property_name] = method_name;
                        }
                    }
                }
            }
        }

        // Now match getters with setters
        for (const auto &[property_name, getter_name] : getters) {
            auto setter_it = setters.find(property_name);

            if (setter_it != setters.end()) {
                // We have both getter and setter!
                const std::string &setter_name = setter_it->second;

                // Verify type compatibility (getter return type == setter param type)
                auto &getter_info = method_info_[getter_name];
                auto &setter_info = method_info_[setter_name];

                if (getter_info.return_type == setter_info.arg_types[0]) {
                    // Types match! Create a virtual property
                    register_property_from_methods(property_name, getter_name, setter_name,
                                                   getter_info.return_type);
                }
            } else {
                // Only getter, create read-only property
                auto &getter_info = method_info_[getter_name];
                register_readonly_property_from_method(property_name, getter_name,
                                                       getter_info.return_type);
            }
        }

        return *this;
    }

    template <typename Class>
    inline std::string ClassMetadata<Class>::to_lower(const std::string &str) {
        std::string result = str;
        for (char &c : result) {
            c = std::tolower(c);
        }
        return result;
    }

    template <typename Class>
    inline void ClassMetadata<Class>::register_property_from_methods(
        const std::string &property_name, const std::string &getter_name,
        const std::string &setter_name, std::type_index value_type) {
        // Only register if not already a field
        if (std::find(field_names_.begin(), field_names_.end(), property_name) !=
            field_names_.end()) {
            return; // Already exists as a field
        }

        field_names_.push_back(property_name);
        field_types_.emplace(property_name, value_type);

        // Create getter that calls the getter method
        field_getters_[property_name] = [this, getter_name](Class &obj) -> Any {
            std::vector<Any> no_args;
            return this->invoke_method(obj, getter_name, no_args);
        };

        // Create setter that calls the setter method
        field_setters_[property_name] = [this, setter_name](Class &obj, Any value) {
            std::vector<Any> args = {value};
            this->invoke_method(obj, setter_name, args);
        };
    }

    template <typename Class>
    inline void
    ClassMetadata<Class>::register_readonly_property_from_method(const std::string &property_name,
                                                                 const std::string &getter_name,
                                                                 std::type_index    value_type) {
        // Only register if not already a field
        if (std::find(field_names_.begin(), field_names_.end(), property_name) !=
            field_names_.end()) {
            return; // Already exists as a field
        }

        field_names_.push_back(property_name);
        field_types_.emplace(property_name, value_type);

        // Create getter that calls the getter method
        field_getters_[property_name] = [this, getter_name](Class &obj) -> Any {
            std::vector<Any> no_args;
            return this->invoke_method(obj, getter_name, no_args);
        };

        // Create setter that throws
        field_setters_[property_name] = [property_name](Class &, Any) {
            throw std::runtime_error("Cannot set read-only property: " + property_name);
        };
    }

    // ========================================================================
    // ACCESSEURS
    // ========================================================================

    template <typename Class>
    inline const std::vector<typename ClassMetadata<Class>::Constructor> &ClassMetadata<Class>::constructors() const {
        return constructors_;
    }

    template <typename Class>
    inline const std::vector<typename ClassMetadata<Class>::ConstructorInfo> &ClassMetadata<Class>::constructor_infos() const {
        return constructor_infos_;
    }

    template <typename Class>
    inline const std::vector<std::string> &ClassMetadata<Class>::fields() const {
        return field_names_;
    }

    template <typename Class>
    inline const std::vector<std::string> &ClassMetadata<Class>::methods() const {
        return method_names_;
    }

    template <typename Class>
    inline Any ClassMetadata<Class>::get_field(Class &obj, const std::string &name) const {
        return field_getters_.at(name)(obj);
    }

    template <typename Class>
    inline void ClassMetadata<Class>::set_field(Class &obj, const std::string &name, Any value) const {
        field_setters_.at(name)(obj, std::move(value));
    }

    template <typename Class>
    inline Any ClassMetadata<Class>::invoke_method(Class &obj, const std::string &name,
                                            std::vector<Any> args) const {
        return methods_.at(name)(obj, std::move(args));
    }

    template <typename Class>
    inline Any ClassMetadata<Class>::invoke_method(const Class &obj, const std::string &name,
                                            std::vector<Any> args) const {
        return const_methods_.at(name)(obj, std::move(args));
    }

    template <typename Class> inline bool ClassMetadata<Class>::is_instantiable() const {
        return !inheritance_.is_abstract &&
               !VirtualMethodRegistry::instance().has_pure_virtual_methods<Class>();
    }

    template <typename Class>
    inline std::type_index ClassMetadata<Class>::get_field_type(const std::string &name) const {
        auto it = field_types_.find(name);
        if (it != field_types_.end()) {
            return it->second;
        }
        return std::type_index(typeid(void));
    }

    template <typename Class>
    template <typename Base>
    inline size_t ClassMetadata<Class>::calculate_base_offset() const {
        if constexpr (std::is_base_of_v<Base, Class> && !std::is_abstract_v<Class>) {
            alignas(Class) char buffer[sizeof(Class)];
            Class              *derived_ptr = reinterpret_cast<Class *>(buffer);
            Base               *base_ptr    = static_cast<Base *>(derived_ptr);
            return reinterpret_cast<char *>(base_ptr) - reinterpret_cast<char *>(derived_ptr);
        }
        return 0;
    }

    // Generate a signature of a method
    template <typename Class>
    template <typename Ret, typename... Args>
    inline std::string ClassMetadata<Class>::make_signature() {
        std::string sig = typeid(Ret).name();
        sig += "(";
        ((sig += typeid(Args).name(), sig += ","), ...);
        if (sizeof...(Args) > 0)
            sig.pop_back();
        sig += ")";
        return sig;
    }

    // To invoke non-const methods with arguments
    template <typename Class>
    template <typename Ret, typename... Args, size_t... Is>
    inline Any ClassMetadata<Class>::invoke_with_args(Class            &obj, Ret (Class::*ptr)(Args...),
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

    // Helper to extract argument with numeric conversion support
    template <typename Class>
    template <typename T>
    inline auto ClassMetadata<Class>::extract_arg(Any &any_val)
        -> std::remove_cv_t<std::remove_reference_t<T>> {
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
    template <typename Class>
    template <typename Ret, typename... Args, size_t... Is>
    inline Any
    ClassMetadata<Class>::invoke_const_with_args(const Class      &obj, Ret (Class::*ptr)(Args...) const,
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

} // namespace rosetta::core