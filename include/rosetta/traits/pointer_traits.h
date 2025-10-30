// ============================================================================
// rosetta/traits/pointer_traits.hpp
//
// Traits pour détecter et analyser les pointeurs et références
// ============================================================================
#pragma once
#include <memory>
#include <type_traits>

namespace rosetta::traits {

    // ============================================================================
    // DÉTECTION DE POINTEURS BRUTS
    // ============================================================================

    /**
     * @brief Détection de pointeurs bruts
     */
    template <typename T> struct is_raw_pointer : std::false_type {};

    template <typename T> struct is_raw_pointer<T *> : std::true_type {};

    template <typename T> inline constexpr bool is_raw_pointer_v = is_raw_pointer<T>::value;

    // ============================================================================
    // DÉTECTION DE SMART POINTERS
    // ============================================================================

    /**
     * @brief Détection de std::shared_ptr
     */
    template <typename T> struct is_shared_ptr : std::false_type {};

    template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

    template <typename T> inline constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

    /**
     * @brief Détection de std::unique_ptr
     */
    template <typename T> struct is_unique_ptr : std::false_type {};

    template <typename T, typename Deleter>
    struct is_unique_ptr<std::unique_ptr<T, Deleter>> : std::true_type {};

    template <typename T> inline constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;

    /**
     * @brief Détection de std::weak_ptr
     */
    template <typename T> struct is_weak_ptr : std::false_type {};

    template <typename T> struct is_weak_ptr<std::weak_ptr<T>> : std::true_type {};

    template <typename T> inline constexpr bool is_weak_ptr_v = is_weak_ptr<T>::value;

    /**
     * @brief Détection générique de smart pointer
     */
    template <typename T>
    inline constexpr bool is_smart_pointer_v =
        is_shared_ptr_v<T> || is_unique_ptr_v<T> || is_weak_ptr_v<T>;

    /**
     * @brief Détection de tout type de pointeur
     */
    template <typename T>
    inline constexpr bool is_pointer_v = is_raw_pointer_v<T> || is_smart_pointer_v<T>;

    // ============================================================================
    // DÉTECTION DE RÉFÉRENCES
    // ============================================================================

    /**
     * @brief Détection de références lvalue
     */
    template <typename T> struct is_lvalue_reference : std::is_lvalue_reference<T> {};

    template <typename T>
    inline constexpr bool is_lvalue_reference_v = is_lvalue_reference<T>::value;

    /**
     * @brief Détection de références rvalue
     */
    template <typename T> struct is_rvalue_reference : std::is_rvalue_reference<T> {};

    template <typename T>
    inline constexpr bool is_rvalue_reference_v = is_rvalue_reference<T>::value;

    /**
     * @brief Détection de tout type de référence
     */
    template <typename T> inline constexpr bool is_reference_v = std::is_reference_v<T>;

    // ============================================================================
    // EXTRACTION DU TYPE POINTÉ
    // ============================================================================

    /**
     * @brief Traits pour extraire le type pointé
     */
    template <typename T> struct pointer_traits {
        using pointee_type                 = void;
        static constexpr bool is_pointer   = false;
        static constexpr bool is_reference = false;
        static constexpr bool is_smart     = false;
    };

    /**
     * @brief Spécialisation pour pointeurs bruts
     */
    template <typename T> struct pointer_traits<T *> {
        using pointee_type                 = T;
        static constexpr bool is_pointer   = true;
        static constexpr bool is_reference = false;
        static constexpr bool is_smart     = false;
    };

    /**
     * @brief Spécialisation pour std::shared_ptr
     */
    template <typename T> struct pointer_traits<std::shared_ptr<T>> {
        using pointee_type                 = T;
        static constexpr bool is_pointer   = true;
        static constexpr bool is_reference = false;
        static constexpr bool is_smart     = true;
    };

    /**
     * @brief Spécialisation pour std::unique_ptr
     */
    template <typename T, typename Deleter> struct pointer_traits<std::unique_ptr<T, Deleter>> {
        using pointee_type                 = T;
        static constexpr bool is_pointer   = true;
        static constexpr bool is_reference = false;
        static constexpr bool is_smart     = true;
    };

    /**
     * @brief Spécialisation pour std::weak_ptr
     */
    template <typename T> struct pointer_traits<std::weak_ptr<T>> {
        using pointee_type                 = T;
        static constexpr bool is_pointer   = true;
        static constexpr bool is_reference = false;
        static constexpr bool is_smart     = true;
    };

    /**
     * @brief Spécialisation pour références lvalue
     */
    template <typename T> struct pointer_traits<T &> {
        using pointee_type                 = T;
        static constexpr bool is_pointer   = false;
        static constexpr bool is_reference = true;
        static constexpr bool is_smart     = false;
    };

    /**
     * @brief Spécialisation pour références rvalue
     */
    template <typename T> struct pointer_traits<T &&> {
        using pointee_type                 = T;
        static constexpr bool is_pointer   = false;
        static constexpr bool is_reference = true;
        static constexpr bool is_smart     = false;
    };

    // ============================================================================
    // HELPERS
    // ============================================================================

    /**
     * @brief Extrait le type pointé
     */
    template <typename T> using pointee_type_t = typename pointer_traits<T>::pointee_type;

    /**
     * @brief Supprime tous les qualificateurs de pointeur/référence
     */
    template <typename T> struct remove_all_pointers {
        using type = T;
    };

    template <typename T> struct remove_all_pointers<T *> {
        using type = typename remove_all_pointers<T>::type;
    };

    template <typename T> struct remove_all_pointers<T &> {
        using type = typename remove_all_pointers<T>::type;
    };

    template <typename T> struct remove_all_pointers<T &&> {
        using type = typename remove_all_pointers<T>::type;
    };

    template <typename T> struct remove_all_pointers<std::shared_ptr<T>> {
        using type = typename remove_all_pointers<T>::type;
    };

    template <typename T, typename D> struct remove_all_pointers<std::unique_ptr<T, D>> {
        using type = typename remove_all_pointers<T>::type;
    };

    template <typename T> struct remove_all_pointers<std::weak_ptr<T>> {
        using type = typename remove_all_pointers<T>::type;
    };

    template <typename T> using remove_all_pointers_t = typename remove_all_pointers<T>::type;

} // namespace rosetta::traits