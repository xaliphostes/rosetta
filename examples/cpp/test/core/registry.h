// ============================================================================
// rosetta/core/registry.hpp
//
// Registry central pour toutes les classes enregistrées
// ============================================================================
#pragma once
#include "class_metadata.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace rosetta::core {

    /**
     * @brief Registry singleton pour toutes les classes enregistrées
     *
     * Ce registry stocke les métadonnées de toutes les classes enregistrées
     * et permet d'y accéder par type ou par nom.
     */
    class Registry {
    public:
        /**
         * @brief Interface de base pour stocker les métadonnées de manière type-erased
         */
        struct MetadataHolder {
            virtual ~MetadataHolder()                              = default;
            virtual std::string            get_name() const        = 0;
            virtual const InheritanceInfo &get_inheritance() const = 0;
        };

        /**
         * @brief Implémentation concrète pour un type spécifique
         */
        template <typename Class> struct MetadataHolderImpl : MetadataHolder {
            ClassMetadata<Class> metadata;

            explicit MetadataHolderImpl(std::string name) : metadata(std::move(name)) {}

            std::string get_name() const override { return metadata.name(); }

            const InheritanceInfo &get_inheritance() const override {
                return metadata.inheritance();
            }
        };

        std::unordered_map<std::string, std::unique_ptr<MetadataHolder>> classes_;
        std::unordered_map<const std::type_info *, std::string>          type_to_name_;

        Registry() = default;

        // Non-copiable, non-movable
        Registry(const Registry &)            = delete;
        Registry &operator=(const Registry &) = delete;

    public:
        /**
         * @brief Obtient l'instance unique du registry
         * @return Référence au registry
         */
        static Registry &instance() {
            static Registry reg;
            return reg;
        }

        /**
         * @brief Enregistre une classe
         * @tparam Class Type de la classe
         * @param name Nom de la classe
         * @return Référence aux métadonnées de la classe
         */
        template <typename Class> ClassMetadata<Class> &register_class(const std::string &name) {
            auto  holder                  = std::make_unique<MetadataHolderImpl<Class>>(name);
            auto *ptr                     = &holder->metadata;
            classes_[name]                = std::move(holder);
            type_to_name_[&typeid(Class)] = name;
            return *ptr;
        }

        /**
         * @brief Obtient les métadonnées d'une classe par son type
         * @tparam Class Type de la classe
         * @return Référence aux métadonnées
         * @throws std::runtime_error si la classe n'est pas enregistrée
         */
        template <typename Class> ClassMetadata<Class> &get() {
            for (auto &[name, holder] : classes_) {
                if (auto *typed = dynamic_cast<MetadataHolderImpl<Class> *>(holder.get())) {
                    return typed->metadata;
                }
            }
            throw std::runtime_error("Class not registered: " + std::string(typeid(Class).name()));
        }

        /**
         * @brief Obtient les métadonnées d'une classe par son type (version const)
         * @tparam Class Type de la classe
         * @return Référence const aux métadonnées
         * @throws std::runtime_error si la classe n'est pas enregistrée
         */
        template <typename Class> const ClassMetadata<Class> &get() const {
            for (const auto &[name, holder] : classes_) {
                if (auto *typed = dynamic_cast<const MetadataHolderImpl<Class> *>(holder.get())) {
                    return typed->metadata;
                }
            }
            throw std::runtime_error("Class not registered: " + std::string(typeid(Class).name()));
        }

        /**
         * @brief Obtient les métadonnées d'une classe par son nom
         * @param name Nom de la classe
         * @return Pointeur vers le holder, ou nullptr si non trouvé
         */
        const MetadataHolder *get_by_name(const std::string &name) const {
            auto it = classes_.find(name);
            return it != classes_.end() ? it->second.get() : nullptr;
        }

        /**
         * @brief Obtient le nom d'une classe depuis son type_info
         * @param type Type de la classe
         * @return Nom de la classe, ou chaîne vide si non enregistrée
         */
        std::string get_class_name(const std::type_info &type) const {
            auto it = type_to_name_.find(&type);
            return it != type_to_name_.end() ? it->second : "";
        }

        /**
         * @brief Liste toutes les classes enregistrées
         * @return Vecteur des noms de classes
         */
        std::vector<std::string> list_classes() const {
            std::vector<std::string> names;
            names.reserve(classes_.size());
            for (const auto &[name, _] : classes_) {
                names.push_back(name);
            }
            return names;
        }

        /**
         * @brief Vérifie si une classe est enregistrée (par nom)
         * @param name Nom de la classe
         * @return true si enregistrée
         */
        bool has_class(const std::string &name) const {
            return classes_.find(name) != classes_.end();
        }

        /**
         * @brief Vérifie si une classe est enregistrée (par type)
         * @tparam Class Type de la classe
         * @return true si enregistrée
         */
        template <typename Class> bool has_class() const {
            return type_to_name_.find(&typeid(Class)) != type_to_name_.end();
        }

        /**
         * @brief Nombre total de classes enregistrées
         * @return Nombre de classes
         */
        size_t size() const { return classes_.size(); }

        /**
         * @brief Supprime toutes les classes enregistrées
         */
        void clear() {
            classes_.clear();
            type_to_name_.clear();
        }
    };

} // namespace rosetta::core