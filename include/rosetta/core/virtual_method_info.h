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

        VirtualMethodInfo(std::string n, std::string sig, bool pure = false)
            : name(std::move(n)), signature(std::move(sig)), is_pure_virtual(pure) {}
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
                                bool is_pure = false) {
            methods.emplace_back(name, signature, is_pure);
            methods.back().vtable_index = methods.size() - 1;
        }

        /**
         * @brief Trouve une méthode virtuelle par son nom
         * @param name Nom de la méthode
         * @return Pointeur vers l'info, ou nullptr si non trouvée
         */
        const VirtualMethodInfo *find_method(const std::string &name) const {
            for (const auto &method : methods) {
                if (method.name == name) {
                    return &method;
                }
            }
            return nullptr;
        }

        /**
         * @brief Trouve une méthode virtuelle par son nom (version non-const)
         */
        VirtualMethodInfo *find_method(const std::string &name) {
            for (auto &method : methods) {
                if (method.name == name) {
                    return &method;
                }
            }
            return nullptr;
        }

        /**
         * @brief Vérifie si la vtable contient des méthodes pures virtuelles
         * @return true s'il y a au moins une méthode pure virtuelle
         */
        bool has_pure_virtual_methods() const {
            for (const auto &method : methods) {
                if (method.is_pure_virtual) {
                    return true;
                }
            }
            return false;
        }
    };

} // namespace rosetta::core