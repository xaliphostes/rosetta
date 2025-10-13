// ============================================================================
// rosetta/traits/container_traits.hpp
//
// Traits pour détecter et analyser les conteneurs STL
// ============================================================================
#pragma once
#include <array>
#include <cstddef>
#include <map>
#include <optional>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace rosetta::traits {

    // ============================================================================
    // DÉTECTION DE CONTENEURS
    // ============================================================================

    /**
     * @brief Détection de std::vector
     */
    template <typename T> struct is_vector : std::false_type {};

    template <typename T> struct is_vector<std::vector<T>> : std::true_type {};

    template <typename T> inline constexpr bool is_vector_v = is_vector<T>::value;

    /**
     * @brief Détection de std::map
     */
    template <typename T> struct is_map : std::false_type {};

    template <typename K, typename V> struct is_map<std::map<K, V>> : std::true_type {};

    template <typename T> inline constexpr bool is_map_v = is_map<T>::value;

    /**
     * @brief Détection de std::unordered_map
     */
    template <typename T> struct is_unordered_map : std::false_type {};

    template <typename K, typename V>
    struct is_unordered_map<std::unordered_map<K, V>> : std::true_type {};

    template <typename T> inline constexpr bool is_unordered_map_v = is_unordered_map<T>::value;

    /**
     * @brief Détection de std::set
     */
    template <typename T> struct is_set : std::false_type {};

    template <typename T> struct is_set<std::set<T>> : std::true_type {};

    template <typename T> inline constexpr bool is_set_v = is_set<T>::value;

    /**
     * @brief Détection de std::unordered_set
     */
    template <typename T> struct is_unordered_set : std::false_type {};

    template <typename T> struct is_unordered_set<std::unordered_set<T>> : std::true_type {};

    template <typename T> inline constexpr bool is_unordered_set_v = is_unordered_set<T>::value;

    /**
     * @brief Détection de std::array
     */
    template <typename T> struct is_array : std::false_type {};

    template <typename T, size_t N> struct is_array<std::array<T, N>> : std::true_type {};

    template <typename T> inline constexpr bool is_array_v = is_array<T>::value;

    /**
     * @brief Détection de std::optional
     */
    template <typename T> struct is_optional : std::false_type {};

    template <typename T> struct is_optional<std::optional<T>> : std::true_type {};

    template <typename T> inline constexpr bool is_optional_v = is_optional<T>::value;

    /**
     * @brief Détection générique de conteneur
     */
    template <typename T>
    inline constexpr bool is_container_v = is_vector_v<T> || is_map_v<T> || is_unordered_map_v<T> ||
                                           is_set_v<T> || is_unordered_set_v<T> || is_array_v<T>;

    // ============================================================================
    // EXTRACTION DES TYPES INTERNES
    // ============================================================================

    /**
     * @brief Traits pour extraire les types internes des conteneurs
     */
    template <typename T> struct container_traits {
        using value_type                   = void;
        using key_type                     = void;
        static constexpr bool is_container = false;
        static constexpr bool has_key      = false;
    };

    /**
     * @brief Spécialisation pour std::vector
     */
    template <typename T> struct container_traits<std::vector<T>> {
        using value_type                          = T;
        using key_type                            = void;
        static constexpr bool        is_container = true;
        static constexpr bool        has_key      = false;
        static constexpr const char *name         = "vector";
    };

    /**
     * @brief Spécialisation pour std::map
     */
    template <typename K, typename V> struct container_traits<std::map<K, V>> {
        using value_type                          = V;
        using key_type                            = K;
        static constexpr bool        is_container = true;
        static constexpr bool        has_key      = true;
        static constexpr const char *name         = "map";
    };

    /**
     * @brief Spécialisation pour std::unordered_map
     */
    template <typename K, typename V> struct container_traits<std::unordered_map<K, V>> {
        using value_type                          = V;
        using key_type                            = K;
        static constexpr bool        is_container = true;
        static constexpr bool        has_key      = true;
        static constexpr const char *name         = "unordered_map";
    };

    /**
     * @brief Spécialisation pour std::set
     */
    template <typename T> struct container_traits<std::set<T>> {
        using value_type                          = T;
        using key_type                            = void;
        static constexpr bool        is_container = true;
        static constexpr bool        has_key      = false;
        static constexpr const char *name         = "set";
    };

    /**
     * @brief Spécialisation pour std::unordered_set
     */
    template <typename T> struct container_traits<std::unordered_set<T>> {
        using value_type                          = T;
        using key_type                            = void;
        static constexpr bool        is_container = true;
        static constexpr bool        has_key      = false;
        static constexpr const char *name         = "unordered_set";
    };

    /**
     * @brief Spécialisation pour std::array
     */
    template <typename T, size_t N> struct container_traits<std::array<T, N>> {
        using value_type                          = T;
        using key_type                            = void;
        static constexpr bool        is_container = true;
        static constexpr bool        has_key      = false;
        static constexpr size_t      size         = N;
        static constexpr const char *name         = "array";
    };

    /**
     * @brief Spécialisation pour std::optional
     */
    template <typename T> struct container_traits<std::optional<T>> {
        using value_type                          = T;
        using key_type                            = void;
        static constexpr bool        is_container = true;
        static constexpr bool        has_key      = false;
        static constexpr const char *name         = "optional";
    };

    // ============================================================================
    // HELPERS
    // ============================================================================

    /**
     * @brief Extrait le type des éléments d'un conteneur
     */
    template <typename T> using container_value_type_t = typename container_traits<T>::value_type;

    /**
     * @brief Extrait le type des clés d'un conteneur associatif
     */
    template <typename T> using container_key_type_t = typename container_traits<T>::key_type;

    /**
     * @brief Vérifie si un conteneur est associatif (a des clés)
     */
    template <typename T>
    inline constexpr bool is_associative_container_v = container_traits<T>::has_key;

} // namespace rosetta::traits