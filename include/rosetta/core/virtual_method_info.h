// ============================================================================
// rosetta/core/virtual_method_info.hpp
//
// Informations sur les méthodes virtuelles et la vtable
// ============================================================================
#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace rosetta::core {

    /**
     * @brief Information sur une méthode virtuelle
     */
    struct VirtualMethodInfo {
        std::string name;
        std::string signature;
        bool        is_pure_virtual = false;
        bool        is_override     = false;
        bool        is_final        = false;
        size_t      vtable_index    = 0;

        VirtualMethodInfo() = default;

        VirtualMethodInfo(std::string n, std::string sig, bool pure = false);
    };

    /**
     * @brief Information sur la table virtuelle d'une classe
     */
    struct VirtualTableInfo {
        std::vector<VirtualMethodInfo> methods;
        size_t                         vtable_ptr_offset = 0;

        /**
         * @brief Ajoute une méthode virtuelle
         */
        void add_virtual_method(const std::string &name, const std::string &signature,
                                bool is_pure = false);

        /**
         * @brief Trouve une méthode virtuelle par son nom
         * @param name Nom de la méthode
         * @return Pointeur vers l'info, ou nullptr si non trouvée
         */
        const VirtualMethodInfo *find_method(const std::string &name) const;

        /**
         * @brief Trouve une méthode virtuelle par son nom (version non-const)
         */
        VirtualMethodInfo *find_method(const std::string &name);

        /**
         * @brief Vérifie si la vtable contient des méthodes pures virtuelles
         * @return true s'il y a au moins une méthode pure virtuelle
         */
        bool has_pure_virtual_methods() const;
    };

} // namespace rosetta::core

#include "inline/virtual_method_info.hxx"
