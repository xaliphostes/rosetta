// ============================================================================
// rosetta/core/registry.hpp
//
// Registry central pour toutes les classes enregistrÃ©es
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
     * Ce registry stocke les mÃ©tadonnÃ©es de toutes les classes enregistrÃ©es
     * et permet d'y accÃ©der par type ou par nom.
     */
    class Registry {
    public:
        /**
         * @brief Interface de base pour stocker les mÃ©tadonnÃ©es de maniÃ¨re type-erased
         */
        struct MetadataHolder {
            virtual ~MetadataHolder()                              = default;
            virtual std::string            get_name() const        = 0;
            virtual const InheritanceInfo &get_inheritance() const = 0;

            // Type-erased method invocation for base class lookup
            virtual bool has_method(const std::string &name) const                 = 0;
            virtual Any  invoke_method_void_ptr(void *obj, const std::string &name,
                                                std::vector<Any> args) const       = 0;
            virtual Any  invoke_const_method_void_ptr(const void *obj, const std::string &name,
                                                      std::vector<Any> args) const = 0;

            // Get list of methods (for dump/introspection)
            virtual std::vector<std::string> get_methods() const = 0;
            
            // Get method metadata for type-erased access
            virtual size_t get_method_arity(const std::string& name) const = 0;
            virtual std::vector<std::type_index> get_method_arg_types(const std::string& name) const = 0;
        };

        /**
         * @brief ImplÃ©mentation concrÃ¨te pour un type spÃ©cifique
         */
        template <typename Class> struct MetadataHolderImpl : MetadataHolder {
            ClassMetadata<Class> metadata;

            explicit MetadataHolderImpl(std::string name) : metadata(std::move(name)) {}

            std::string get_name() const override { return metadata.name(); }

            const InheritanceInfo &get_inheritance() const override {
                return metadata.inheritance();
            }

            bool has_method(const std::string &name) const override {
                const auto &methods = metadata.methods();
                return std::find(methods.begin(), methods.end(), name) != methods.end();
            }

            Any invoke_method_void_ptr(void *obj, const std::string &name,
                                       std::vector<Any> args) const override {
                Class *typed_obj = static_cast<Class *>(obj);
                return metadata.invoke_method(*typed_obj, name, std::move(args));
            }

            Any invoke_const_method_void_ptr(const void *obj, const std::string &name,
                                             std::vector<Any> args) const override {
                const Class *typed_obj = static_cast<const Class *>(obj);
                return metadata.invoke_method(*typed_obj, name, std::move(args));
            }

            std::vector<std::string> get_methods() const override { return metadata.methods(); }
            
            size_t get_method_arity(const std::string& name) const override {
                return metadata.get_method_arity(name);
            }
            
            std::vector<std::type_index> get_method_arg_types(const std::string& name) const override {
                return metadata.get_method_arg_types(name);
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
         * @return RÃ©fÃ©rence au registry
         */
        static Registry &instance();

        /**
         * @brief Enregistre une classe
         * @tparam Class Type de la classe
         * @param name Nom de la classe
         * @return RÃ©fÃ©rence aux mÃ©tadonnÃ©es de la classe
         */
        template <typename Class> ClassMetadata<Class> &register_class(const std::string &name);

        /**
         * @brief Obtient les mÃ©tadonnÃ©es d'une classe par son type
         * @tparam Class Type de la classe
         * @return RÃ©fÃ©rence aux mÃ©tadonnÃ©es
         * @throws std::runtime_error si la classe n'est pas enregistrÃ©e
         */
        template <typename Class> ClassMetadata<Class> &get();

        /**
         * @brief Obtient les mÃ©tadonnÃ©es d'une classe par son type (version const)
         * @tparam Class Type de la classe
         * @return RÃ©fÃ©rence const aux mÃ©tadonnÃ©es
         * @throws std::runtime_error si la classe n'est pas enregistrÃ©e
         */
        template <typename Class> const ClassMetadata<Class> &get() const;

        /**
         * @brief Obtient les mÃ©tadonnÃ©es d'une classe par son nom
         * @param name Nom de la classe
         * @return Pointeur vers le holder, ou nullptr si non trouvÃ©
         */
        const MetadataHolder *get_by_name(const std::string &name) const;

        /**
         * @brief Obtient le nom d'une classe depuis son type_info
         * @param type Type de la classe
         * @return Nom de la classe, ou chaÃ®ne vide si non enregistrÃ©e
         */
        std::string get_class_name(const std::type_info &type) const;

        /**
         * @brief Liste toutes les classes enregistrÃ©es
         * @return Vecteur des noms de classes
         */
        std::vector<std::string> list_classes() const;

        /**
         * @brief VÃ©rifie si une classe est enregistrÃ©e (par nom)
         * @param name Nom de la classe
         * @return true si enregistrÃ©e
         */
        bool has_class(const std::string &name) const;

        /**
         * @brief VÃ©rifie si une classe est enregistrÃ©e (par type)
         * @tparam Class Type de la classe
         * @return true si enregistrÃ©e
         */
        template <typename Class> bool has_class() const;

        /**
         * @brief Nombre total de classes enregistrÃ©es
         * @return Nombre de classes
         */
        size_t size() const;

        /**
         * @brief Supprime toutes les classes enregistrÃ©es
         */
        void clear();
    };

} // namespace rosetta::core

#include "inline/registry.hxx"