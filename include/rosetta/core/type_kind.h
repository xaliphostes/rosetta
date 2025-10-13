// ============================================================================
// rosetta/core/type_kind.hpp
//
// Définit les énumérations de base pour la bibliothèque Rosetta
// ============================================================================
#pragma once

namespace rosetta::core {

    /**
     * @brief Catégories de types supportés par Rosetta
     */
    enum class TypeKind {
        // Types primitifs
        Void,
        Bool,
        Int,
        Float,
        Double,
        String,

        // Conteneurs STL
        Vector,
        Map,
        Set,
        Array,
        Optional,

        // Pointeurs et références
        RawPointer,
        SharedPtr,
        UniquePtr,
        WeakPtr,
        Reference,

        // Types custom
        Object
    };

    /**
     * @brief Spécificateurs d'accès pour les membres de classe
     */
    enum class AccessSpecifier { Public, Protected, Private };

    /**
     * @brief Type d'héritage
     */
    enum class InheritanceType { Normal, Virtual };

} // namespace rosetta::core