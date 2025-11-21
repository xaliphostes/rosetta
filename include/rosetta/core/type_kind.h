// ============================================================================
// Définit les énumérations de base pour la bibliothèque Rosetta
// ============================================================================
#pragma once

namespace rosetta::core {

    /**
     * @brief Category of types supported by Rosetta
     */
    enum class TypeKind {
        // Primitif types
        Void,
        Bool,
        Int,
        Float,
        Double,
        String,

        // STL conteners
        Vector,
        Map,
        Set,
        Array,
        Optional,

        // Pointeurs and references
        RawPointer,
        SharedPtr,
        UniquePtr,
        WeakPtr,
        Reference,

        // Types custom
        Object
    };

    enum class AccessSpecifier { Public, Protected, Private };
    enum class InheritanceType { Normal, Virtual };

} // namespace rosetta::core