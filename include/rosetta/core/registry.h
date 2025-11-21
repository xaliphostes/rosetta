// ============================================================================
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
     * This registry store metadata of all registered classes
     * and allows access by type or by name
     */
    class Registry {
    public:
        /**
         * @brief Base interface
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
            virtual size_t get_method_arity(const std::string &name) const = 0;
            virtual std::vector<std::type_index>
            get_method_arg_types(const std::string &name) const = 0;
        };

        /**
         * @brief Concrete impl
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

            size_t get_method_arity(const std::string &name) const override {
                return metadata.get_method_arity(name);
            }

            std::vector<std::type_index>
            get_method_arg_types(const std::string &name) const override {
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
        static Registry &instance();
        template <typename Class> ClassMetadata<Class> &register_class(const std::string &name);
        template <typename Class> ClassMetadata<Class> &get();
        template <typename Class> const ClassMetadata<Class> &get() const;
        const MetadataHolder *get_by_name(const std::string &name) const;
        std::string get_class_name(const std::type_info &type) const;
        std::vector<std::string> list_classes() const;
        bool has_class(const std::string &name) const;
        template <typename Class> bool has_class() const;
        size_t size() const;
        void clear();
    };

} // namespace rosetta::core

#include "inline/registry.hxx"