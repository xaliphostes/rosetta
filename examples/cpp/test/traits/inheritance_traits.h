// ============================================================================
// rosetta/traits/inheritance_traits.hpp
//
// Traits pour détecter et analyser l'héritage
// ============================================================================
#pragma once
#include <cstddef>
#include <type_traits>

namespace rosetta::traits {

    /**
     * @brief Vérifie si Derived hérite de Base
     */
    template <typename Derived, typename Base>
    inline constexpr bool is_derived_from_v = std::is_base_of_v<Base, Derived>;

    /**
     * @brief Vérifie si une classe est abstraite
     */
    template <typename T> inline constexpr bool is_abstract_v = std::is_abstract_v<T>;

    /**
     * @brief Vérifie si une classe est polymorphique (a une vtable)
     */
    template <typename T> inline constexpr bool is_polymorphic_v = std::is_polymorphic_v<T>;

    /**
     * @brief Vérifie si une classe a un destructeur virtuel
     */
    template <typename T>
    inline constexpr bool has_virtual_destructor_v = std::has_virtual_destructor_v<T>;

    /**
     * @brief Calcule l'offset mémoire d'une classe de base
     * @tparam Derived Classe dérivée
     * @tparam Base Classe de base
     * @return Offset en bytes
     */
    template <typename Derived, typename Base> size_t base_offset() {
        if constexpr (std::is_base_of_v<Base, Derived> && !std::is_abstract_v<Derived>) {
            alignas(Derived) char buffer[sizeof(Derived)];
            Derived              *derived_ptr = reinterpret_cast<Derived *>(buffer);
            Base                 *base_ptr    = static_cast<Base *>(derived_ptr);
            return reinterpret_cast<char *>(base_ptr) - reinterpret_cast<char *>(derived_ptr);
        }
        return 0;
    }

    /**
     * @brief Obtient le type dynamique d'un objet via RTTI
     * @tparam Base Type de base (doit être polymorphique)
     * @param obj Référence à l'objet
     * @return type_info du type réel
     */
    template <typename Base> const std::type_info &dynamic_type(const Base &obj) {
        if constexpr (std::is_polymorphic_v<Base>) {
            return typeid(obj); // Utilise RTTI
        } else {
            return typeid(Base); // Type statique seulement
        }
    }

    /**
     * @brief Vérifie si une méthode pourrait être virtuelle
     * @note Approximation basée sur le polymorphisme de la classe
     */
    template <typename Class, typename Ret, typename... Args>
    constexpr bool is_method_potentially_virtual(Ret (Class::*)(Args...)) {
        return std::is_polymorphic_v<Class>;
    }

    /**
     * @brief Traits pour obtenir des informations sur l'héritage
     */
    template <typename T> struct inheritance_traits {
        static constexpr bool is_polymorphic         = std::is_polymorphic_v<T>;
        static constexpr bool is_abstract            = std::is_abstract_v<T>;
        static constexpr bool has_virtual_destructor = std::has_virtual_destructor_v<T>;
        static constexpr bool is_final               = std::is_final_v<T>;
    };

    /**
     * @brief Vérifie si T hérite virtuellement de Base (approximation)
     * @note Détection exacte impossible sans introspection du compilateur
     */
    template <typename Derived, typename Base> struct has_virtual_base : std::false_type {};

    /**
     * @brief Helper pour détecter l'héritage en diamant
     * @note Vérifie si deux bases partagent une base commune
     */
    template <typename Derived, typename Base1, typename Base2, typename Common>
    struct has_diamond_inheritance
        : std::bool_constant<std::is_base_of_v<Common, Base1> && std::is_base_of_v<Common, Base2> &&
                             std::is_base_of_v<Base1, Derived> &&
                             std::is_base_of_v<Base2, Derived>> {};

    template <typename Derived, typename Base1, typename Base2, typename Common>
    inline constexpr bool has_diamond_inheritance_v =
        has_diamond_inheritance<Derived, Base1, Base2, Common>::value;

} // namespace rosetta::traits