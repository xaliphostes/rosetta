// ============================================================================
// Informations sur l'héritage des classes
// ============================================================================
#pragma once
#include "type_kind.h"
#include "virtual_method_info.h"
#include <cstddef>
#include <string>
#include <typeinfo>
#include <vector>

namespace rosetta::core {

    /**
     * @brief Information sur une classe de base
     */
    struct BaseClassInfo {
        std::string           name;
        const std::type_info *type;
        InheritanceType       inheritance_type;
        AccessSpecifier       access;
        size_t                offset;
        bool                  is_primary_base = false;

        BaseClassInfo(std::string, const std::type_info *, InheritanceType, AccessSpecifier,
                      size_t);
    };

    /**
     * @brief Information complète sur l'héritage d'une classe
     */
    struct InheritanceInfo {
        std::vector<BaseClassInfo> base_classes;
        std::vector<BaseClassInfo> virtual_bases;

        bool is_abstract            = false;
        bool is_polymorphic         = false;
        bool has_virtual_destructor = false;

        VirtualTableInfo vtable;

        /**
         * @brief Ajoute une classe de base
         */
        void add_base(const std::string &name, const std::type_info *type,
                      InheritanceType itype  = InheritanceType::Normal,
                      AccessSpecifier access = AccessSpecifier::Public, size_t offset = 0);

        /**
         * @brief Vérifie si une classe hérite d'un type donné
         * @param type Type à vérifier
         * @return true si c'est une classe de base
         */
        bool has_base(const std::type_info &type) const;

        /**
         * @brief Obtient l'info d'une classe de base
         * @param type Type recherché
         * @return Pointeur vers l'info, ou nullptr si non trouvée
         */
        const BaseClassInfo *get_base(const std::type_info &type) const;

        /**
         * @brief Compte le nombre total de bases (normales + virtuelles)
         * @return Nombre de classes de base
         */
        size_t total_base_count() const;
    };

} // namespace rosetta::core

#include "inline/inheritance_info.hxx"
