// ============================================================================
// rosetta/core/class_metadata.hpp
//
// MÃƒÆ’Ã‚Â©tadonnÃƒÆ’Ã‚Â©es complÃƒÆ’Ã‚Â¨tes pour une classe : champs, mÃƒÆ’Ã‚Â©thodes, hÃƒÆ’Ã‚Â©ritage
// ============================================================================
#pragma once
#include "any.h"
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

    /**
     * @brief MÃƒÆ’Ã‚Â©tadonnÃƒÆ’Ã‚Â©es complÃƒÆ’Ã‚Â¨tes pour une classe
     * @tparam Class Type de la classe
     */
    template <typename Class> class ClassMetadata {
        std::string     name_;
        InheritanceInfo inheritance_;

        std::unordered_map<std::string, std::function<Any(Class &)>>       field_getters_;
        std::unordered_map<std::string, std::function<void(Class &, Any)>> field_setters_;
        std::unordered_map<std::string, std::function<Any(Class &, std::vector<Any>)>> methods_;
        std::unordered_map<std::string, std::function<Any(const Class &, std::vector<Any>)>>
            const_methods_;

        std::vector<std::string> field_names_;
        std::vector<std::string> method_names_;

        // Store type information for fields (for JS binding)
        std::unordered_map<std::string, std::type_index> field_types_;

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
         * @brief Obtient le nom de la classe
         */
        const std::string &name() const { return name_; }

        /**
         * @brief Obtient les informations d'hÃƒÆ’Ã‚Â©ritage
         */
        const InheritanceInfo &inheritance() const { return inheritance_; }

        /**
         * @brief Obtient les informations d'hÃƒÆ’Ã‚Â©ritage (version modifiable)
         */
        InheritanceInfo &inheritance() { return inheritance_; }

        // ========================================================================
        // DÃƒÆ’Ã¢â‚¬Â°CLARATION D'HÃƒÆ’Ã¢â‚¬Â°RITAGE
        // ========================================================================

        /**
         * @brief DÃƒÆ’Ã‚Â©clare qu'une classe hÃƒÆ’Ã‚Â©rite d'une classe de base
         * @tparam Base Type de la classe de base
         * @param base_name Nom de la base (optionnel)
         * @param access SpÃƒÆ’Ã‚Â©cificateur d'accÃƒÆ’Ã‚Â¨s
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
         * @brief DÃƒÆ’Ã‚Â©clare un hÃƒÆ’Ã‚Â©ritage virtuel
         * @tparam Base Type de la classe de base
         * @param base_name Nom de la base (optionnel)
         * @param access SpÃƒÆ’Ã‚Â©cificateur d'accÃƒÆ’Ã‚Â¨s
         */
        template <typename Base>
        ClassMetadata &virtually_inherits_from(const std::string &base_name = "",
                                               AccessSpecifier access = AccessSpecifier::Public) {
            static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

            std::string name   = base_name.empty() ? typeid(Base).name() : base_name;
            size_t      offset = 0; // Offset dÃƒÆ’Ã‚Â©terminÃƒÆ’Ã‚Â© ÃƒÆ’Ã‚Â  l'exÃƒÆ’Ã‚Â©cution pour hÃƒÆ’Ã‚Â©ritage virtuel

            inheritance_.add_base(name, &typeid(Base), InheritanceType::Virtual, access, offset);

            return *this;
        }

        // ========================================================================
        // ENREGISTREMENT DE CHAMPS
        // ========================================================================

        /**
         * @brief Enregistre un champ de la classe
         * @tparam T Type du champ
         * @param name Nom du champ
         * @param ptr Pointeur vers membre
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
        // ENREGISTREMENT DE MÃƒÆ’Ã¢â‚¬Â°THODES NON-CONST
        // ========================================================================

        /**
         * @brief Enregistre une mÃƒÆ’Ã‚Â©thode non-const
         * @tparam Ret Type de retour
         * @tparam Args Types des paramÃƒÆ’Ã‚Â¨tres
         * @param name Nom de la mÃƒÆ’Ã‚Â©thode
         * @param ptr Pointeur vers la mÃƒÆ’Ã‚Â©thode
         */
        template <typename Ret, typename... Args>
        ClassMetadata &method(const std::string &name, Ret (Class::*ptr)(Args...)) {
            method_names_.push_back(name);

            methods_[name] = [ptr](Class &obj, std::vector<Any> args) -> Any {
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

            return *this;
        }

        // ========================================================================
        // ENREGISTREMENT DE MÃƒÆ’Ã¢â‚¬Â°THODES CONST
        // ========================================================================

        /**
         * @brief Enregistre une mÃƒÆ’Ã‚Â©thode const
         * @tparam Ret Type de retour
         * @tparam Args Types des paramÃƒÆ’Ã‚Â¨tres
         * @param name Nom de la mÃƒÆ’Ã‚Â©thode
         * @param ptr Pointeur vers la mÃƒÆ’Ã‚Â©thode const
         */
        template <typename Ret, typename... Args>
        ClassMetadata &method(const std::string &name, Ret (Class::*ptr)(Args...) const) {
            method_names_.push_back(name);

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
            methods_[name] = [ptr](Class &obj, std::vector<Any> args) -> Any {
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

            return *this;
        }

        // ========================================================================
        // MÃƒÆ’Ã¢â‚¬Â°THODES DE CLASSE DE BASE
        // ========================================================================

        /**
         * @brief Enregistre une mÃƒÆ’Ã‚Â©thode non-const d'une classe de base
         */
        template <typename Base, typename Ret, typename... Args>
        ClassMetadata &base_method(const std::string &name, Ret (Base::*ptr)(Args...)) {
            static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

            method_names_.push_back(name);

            methods_[name] = [ptr](Class &obj, std::vector<Any> args) -> Any {
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

            return *this;
        }

        /**
         * @brief Enregistre une mÃƒÆ’Ã‚Â©thode const d'une classe de base
         */
        template <typename Base, typename Ret, typename... Args>
        ClassMetadata &base_method(const std::string &name, Ret (Base::*ptr)(Args...) const) {
            static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

            method_names_.push_back(name);

            const_methods_[name] = [ptr](const Class &obj, std::vector<Any> args) -> Any {
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

            methods_[name] = [ptr](Class &obj, std::vector<Any> args) -> Any {
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

            return *this;
        }

        // ========================================================================
        // MÃƒÆ’Ã¢â‚¬Â°THODES VIRTUELLES
        // ========================================================================

        /**
         * @brief Enregistre une mÃƒÆ’Ã‚Â©thode virtuelle
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
         * @brief Enregistre une mÃƒÆ’Ã‚Â©thode virtuelle const
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
         * @brief Enregistre une mÃƒÆ’Ã‚Â©thode pure virtuelle
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
         * @brief Enregistre une mÃƒÆ’Ã‚Â©thode qui override une mÃƒÆ’Ã‚Â©thode de base
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
         * @brief Enregistre une mÃƒÆ’Ã‚Â©thode const qui override
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

        /**
         * @brief Liste des noms de champs
         */
        const std::vector<std::string> &fields() const { return field_names_; }

        /**
         * @brief Liste des noms de mÃƒÆ’Ã‚Â©thodes
         */
        const std::vector<std::string> &methods() const { return method_names_; }

        /**
         * @brief RÃƒÆ’Ã‚Â©cupÃƒÆ’Ã‚Â¨re la valeur d'un champ
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
         * @brief Invoque une mÃƒÆ’Ã‚Â©thode
         */
        Any invoke_method(Class &obj, const std::string &name, std::vector<Any> args = {}) const {
            return methods_.at(name)(obj, std::move(args));
        }

        /**
         * @brief Invoque une mÃƒÆ’Ã‚Â©thode sur un objet const
         */
        Any invoke_method(const Class &obj, const std::string &name,
                          std::vector<Any> args = {}) const {
            return const_methods_.at(name)(obj, std::move(args));
        }

        /**
         * @brief VÃƒÆ’Ã‚Â©rifie si la classe peut ÃƒÆ’Ã‚Âªtre instanciÃƒÆ’Ã‚Â©e
         */
        bool is_instantiable() const {
            return !inheritance_.is_abstract &&
                   !VirtualMethodRegistry::instance().has_pure_virtual_methods<Class>();
        }

        /**
         * @brief Obtient le type_index d'un champ
         * @param name Nom du champ
         * @return type_index du champ, ou typeid(void) si non trouvÃƒÂ©
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

        // GÃƒÆ’Ã‚Â©nÃƒÆ’Ã‚Â¨re une signature de mÃƒÆ’Ã‚Â©thode
        template <typename Ret, typename... Args> static std::string make_signature() {
            std::string sig = typeid(Ret).name();
            sig += "(";
            ((sig += typeid(Args).name(), sig += ","), ...);
            if (sizeof...(Args) > 0)
                sig.pop_back();
            sig += ")";
            return sig;
        }

        // Helper pour invoquer mÃƒÆ’Ã‚Â©thodes non-const
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
            
            std::type_index actual_type = any_val.get_type_index();
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

        // Helper pour invoquer mÃƒÆ’Ã‚Â©thodes const
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