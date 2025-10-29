// ============================================================================
// rosetta/core/class_metadata.hpp
//
// MÃ©tadonnÃ©es complÃ¨tes pour une classe : champs, mÃ©thodes, hÃ©ritage
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
     * @brief MÃ©tadonnÃ©es complÃ¨tes pour une classe
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
         * @brief Obtient les informations d'hÃ©ritage
         */
        const InheritanceInfo &inheritance() const { return inheritance_; }

        /**
         * @brief Obtient les informations d'hÃ©ritage (version modifiable)
         */
        InheritanceInfo &inheritance() { return inheritance_; }

        // ========================================================================
        // DÃ‰CLARATION D'HÃ‰RITAGE
        // ========================================================================

        /**
         * @brief DÃ©clare qu'une classe hÃ©rite d'une classe de base
         * @tparam Base Type de la classe de base
         * @param base_name Nom de la base (optionnel)
         * @param access SpÃ©cificateur d'accÃ¨s
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
         * @brief DÃ©clare un hÃ©ritage virtuel
         * @tparam Base Type de la classe de base
         * @param base_name Nom de la base (optionnel)
         * @param access SpÃ©cificateur d'accÃ¨s
         */
        template <typename Base>
        ClassMetadata &virtually_inherits_from(const std::string &base_name = "",
                                               AccessSpecifier access = AccessSpecifier::Public) {
            static_assert(std::is_base_of_v<Base, Class>, "Base must be a base class of Class");

            std::string name   = base_name.empty() ? typeid(Base).name() : base_name;
            size_t      offset = 0; // Offset dÃ©terminÃ© Ã  l'exÃ©cution pour hÃ©ritage virtuel

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
        // ENREGISTREMENT DE MÃ‰THODES NON-CONST
        // ========================================================================

        /**
         * @brief Enregistre une mÃ©thode non-const
         * @tparam Ret Type de retour
         * @tparam Args Types des paramÃ¨tres
         * @param name Nom de la mÃ©thode
         * @param ptr Pointeur vers la mÃ©thode
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
        // ENREGISTREMENT DE MÃ‰THODES CONST
        // ========================================================================

        /**
         * @brief Enregistre une mÃ©thode const
         * @tparam Ret Type de retour
         * @tparam Args Types des paramÃ¨tres
         * @param name Nom de la mÃ©thode
         * @param ptr Pointeur vers la mÃ©thode const
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
        // MÃ‰THODES DE CLASSE DE BASE
        // ========================================================================

        /**
         * @brief Enregistre une mÃ©thode non-const d'une classe de base
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
         * @brief Enregistre une mÃ©thode const d'une classe de base
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
        // MÃ‰THODES VIRTUELLES
        // ========================================================================

        /**
         * @brief Enregistre une mÃ©thode virtuelle
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
         * @brief Enregistre une mÃ©thode virtuelle const
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
         * @brief Enregistre une mÃ©thode pure virtuelle
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
         * @brief Enregistre une mÃ©thode qui override une mÃ©thode de base
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
         * @brief Enregistre une mÃ©thode const qui override
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
         * @brief Liste des noms de mÃ©thodes
         */
        const std::vector<std::string> &methods() const { return method_names_; }

        /**
         * @brief RÃ©cupÃ¨re la valeur d'un champ
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
         * @brief Invoque une mÃ©thode
         */
        Any invoke_method(Class &obj, const std::string &name, std::vector<Any> args = {}) const {
            return methods_.at(name)(obj, std::move(args));
        }

        /**
         * @brief Invoque une mÃ©thode sur un objet const
         */
        Any invoke_method(const Class &obj, const std::string &name,
                          std::vector<Any> args = {}) const {
            return const_methods_.at(name)(obj, std::move(args));
        }

        /**
         * @brief VÃ©rifie si la classe peut Ãªtre instanciÃ©e
         */
        bool is_instantiable() const {
            return !inheritance_.is_abstract &&
                   !VirtualMethodRegistry::instance().has_pure_virtual_methods<Class>();
        }

        /**
         * @brief Obtient le type_index d'un champ
         * @param name Nom du champ
         * @return type_index du champ, ou typeid(void) si non trouvé
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

        // GÃ©nÃ¨re une signature de mÃ©thode
        template <typename Ret, typename... Args> static std::string make_signature() {
            std::string sig = typeid(Ret).name();
            sig += "(";
            ((sig += typeid(Args).name(), sig += ","), ...);
            if (sizeof...(Args) > 0)
                sig.pop_back();
            sig += ")";
            return sig;
        }

        // Helper pour invoquer mÃ©thodes non-const
        template <typename Ret, typename... Args, size_t... Is>
        static Any invoke_with_args(Class &obj, Ret (Class::*ptr)(Args...), std::vector<Any> &args,
                                    std::index_sequence<Is...>) {
            if constexpr (std::is_void_v<Ret>) {
                (obj.*ptr)(args[Is].as<Args>()...);
                return Any(0);
            } else {
                return Any((obj.*ptr)(args[Is].as<Args>()...));
            }
        }

        // Helper pour invoquer mÃ©thodes const
        template <typename Ret, typename... Args, size_t... Is>
        static Any invoke_const_with_args(const Class      &obj, Ret (Class::*ptr)(Args...) const,
                                          std::vector<Any> &args, std::index_sequence<Is...>) {
            if constexpr (std::is_void_v<Ret>) {
                (obj.*ptr)(args[Is].as<Args>()...);
                return Any(0);
            } else {
                return Any((obj.*ptr)(args[Is].as<Args>()...));
            }
        }
    };

} // namespace rosetta::core