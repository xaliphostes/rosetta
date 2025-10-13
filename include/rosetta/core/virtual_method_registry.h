// ============================================================================
// rosetta/core/virtual_method_registry.hpp
//
// Registry global pour les informations sur les méthodes virtuelles
// ============================================================================
#pragma once
#include "virtual_method_info.h"
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace rosetta::core {

    /**
     * @brief Registry singleton pour stocker les informations sur les vtables
     */
    class VirtualMethodRegistry {
        struct ClassVTable {
            VirtualTableInfo                                         vtable;
            std::unordered_map<std::string, std::function<void *()>> method_thunks;
        };

        std::unordered_map<std::string, ClassVTable> class_vtables_;

        VirtualMethodRegistry() = default;

        // Non-copiable, non-movable
        VirtualMethodRegistry(const VirtualMethodRegistry &)            = delete;
        VirtualMethodRegistry &operator=(const VirtualMethodRegistry &) = delete;

    public:
        /**
         * @brief Obtient l'instance unique du registry
         * @return Référence au registry
         */
        static VirtualMethodRegistry &instance() {
            static VirtualMethodRegistry registry;
            return registry;
        }

        /**
         * @brief Enregistre une méthode virtuelle pour une classe
         * @tparam Class Type de la classe
         * @param method_name Nom de la méthode
         * @param signature Signature de la méthode
         * @param is_pure true si la méthode est pure virtuelle
         */
        template <typename Class>
        void register_virtual_method(const std::string &method_name, const std::string &signature,
                                     bool is_pure = false) {
            std::string class_name = typeid(Class).name();
            auto       &vtable     = class_vtables_[class_name].vtable;
            vtable.add_virtual_method(method_name, signature, is_pure);
        }

        /**
         * @brief Obtient la vtable d'une classe
         * @tparam Class Type de la classe
         * @return Pointeur vers la vtable, ou nullptr si non trouvée
         */
        template <typename Class> const VirtualTableInfo *get_vtable() const {
            std::string class_name = typeid(Class).name();
            auto        it         = class_vtables_.find(class_name);
            return it != class_vtables_.end() ? &it->second.vtable : nullptr;
        }

        /**
         * @brief Obtient la vtable d'une classe (version non-const)
         * @tparam Class Type de la classe
         * @return Pointeur vers la vtable, ou nullptr si non trouvée
         */
        template <typename Class> VirtualTableInfo *get_vtable() {
            std::string class_name = typeid(Class).name();
            auto        it         = class_vtables_.find(class_name);
            return it != class_vtables_.end() ? &it->second.vtable : nullptr;
        }

        /**
         * @brief Vérifie si une classe a des méthodes pures virtuelles
         * @tparam Class Type de la classe
         * @return true si la classe a au moins une méthode pure virtuelle
         */
        template <typename Class> bool has_pure_virtual_methods() const {
            auto *vtable = get_vtable<Class>();
            if (!vtable) {
                return false;
            }
            return vtable->has_pure_virtual_methods();
        }

        /**
         * @brief Enregistre un thunk pour appeler une méthode virtuelle
         * @tparam Class Type de la classe
         * @param method_name Nom de la méthode
         * @param thunk Fonction pour appeler la méthode
         */
        template <typename Class>
        void register_method_thunk(const std::string &method_name, std::function<void *()> thunk) {
            std::string class_name                                = typeid(Class).name();
            class_vtables_[class_name].method_thunks[method_name] = std::move(thunk);
        }

        /**
         * @brief Efface toutes les informations enregistrées
         */
        void clear() { class_vtables_.clear(); }
    };

} // namespace rosetta::core