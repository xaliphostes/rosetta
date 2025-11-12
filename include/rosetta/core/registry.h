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
     * @brief Registry singleton for all registered classes
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
        static Registry &instance();

        /**
         * @brief Enregistre une classe
         * @tparam Class Type de la classe
         * @param name Nom de la classe
         * @return Référence aux métadonnées de la classe
         */
        template <typename Class> ClassMetadata<Class> &register_class(const std::string &name);

        /**
         * @brief Obtient les métadonnées d'une classe par son type
         * @tparam Class Type de la classe
         * @return Référence aux métadonnées
         * @throws std::runtime_error si la classe n'est pas enregistrée
         */
        template <typename Class> ClassMetadata<Class> &get();

        /**
         * @brief Obtient les métadonnées d'une classe par son type (version const)
         * @tparam Class Type de la classe
         * @return Référence const aux métadonnées
         * @throws std::runtime_error si la classe n'est pas enregistrée
         */
        template <typename Class> const ClassMetadata<Class> &get() const;

        /**
         * @brief Obtient les métadonnées d'une classe par son nom
         * @param name Nom de la classe
         * @return Pointeur vers le holder, ou nullptr si non trouvé
         */
        const MetadataHolder *get_by_name(const std::string &name) const;

        /**
         * @brief Obtient le nom d'une classe depuis son type_info
         * @param type Type de la classe
         * @return Nom de la classe, ou chaîne vide si non enregistrée
         */
        std::string get_class_name(const std::type_info &type) const;

        /**
         * @brief Liste toutes les classes enregistrées
         * @return Vecteur des noms de classes
         */
        std::vector<std::string> list_classes() const;

        /**
         * @brief Vérifie si une classe est enregistrée (par nom)
         * @param name Nom de la classe
         * @return true si enregistrée
         */
        bool has_class(const std::string &name) const;

        /**
         * @brief Vérifie si une classe est enregistrée (par type)
         * @tparam Class Type de la classe
         * @return true si enregistrée
         */
        template <typename Class> bool has_class() const;

        /**
         * @brief Nombre total de classes enregistrées
         * @return Nombre de classes
         */
        size_t size() const;

        /**
         * @brief Supprime toutes les classes enregistrées
         */
        void clear();
    };

} // namespace rosetta::core

#include "inline/registry.hxx"
