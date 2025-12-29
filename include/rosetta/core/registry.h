// ============================================================================
// Registry central pour toutes les classes enregistrées
// EXTENDED WITH PROPERTY SUPPORT - Differentiates fields from properties
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
         * @brief Base interface for type-erased class metadata access
         */
        struct MetadataHolder {
            virtual ~MetadataHolder()                              = default;
            virtual std::string            get_name() const        = 0;
            virtual const InheritanceInfo &get_inheritance() const = 0;

            // ================================================================
            // Type-erased constructor access
            // ================================================================

            /// Constructor metadata for type-erased access
            struct ConstructorMeta {
                std::vector<std::type_index> param_types;
                size_t                       arity = 0;

                /// Get parameter types as demangled strings
                std::vector<std::string> get_param_types() const;
            };

            /// Get list of all constructors with their parameter types
            virtual std::vector<ConstructorMeta> get_constructors() const = 0;

            /// Get the C++ type name (demangled)
            virtual std::string get_cpp_type_name() const = 0;

            /// Get base class name (empty if no base class)
            virtual std::string get_base_class() const = 0;

            /// Method metadata for type-erased access
            struct MethodMeta {
                std::vector<std::type_index> param_types;
                std::type_index              return_type = std::type_index(typeid(void));
                size_t                       arity       = 0;
                bool                         is_const    = false;

                /// Get parameter types as demangled strings
                std::vector<std::string> get_param_types_str() const;

                /// Get return type as demangled string
                std::string get_return_type_str() const { return demangle(return_type.name()); }
            };

            /// Get detailed method info by name
            virtual MethodMeta get_method_info(const std::string &name) const = 0;

            // ================================================================
            // Property metadata (virtual fields via getter/setter)
            // ================================================================

            /// Property metadata for type-erased access
            struct PropertyMeta {
                std::string     name;           // Property name (e.g., "points")
                std::string     getter_name;    // Getter method name (e.g., "getPoints") - may be empty
                std::string     setter_name;    // Setter method name (e.g., "setPoints") - may be empty
                std::type_index value_type = std::type_index(typeid(void));
                bool            is_readonly  = false;
                bool            is_writeonly = false;

                /// Get value type as demangled string
                std::string get_value_type_str() const;
            };

            // ================================================================
            // Type-erased field access (actual C++ member fields)
            // ================================================================

            /// Check if a field exists
            virtual bool has_field(const std::string &name) const = 0;

            /// Get list of all field names (actual C++ member fields only)
            virtual std::vector<std::string> get_fields() const = 0;

            /// Get field type
            virtual std::type_index get_field_type(const std::string &name) const = 0;

            /// Get field value (type-erased via void*)
            virtual Any get_field_void_ptr(void *obj, const std::string &name) const = 0;

            /// Get field value from const object
            virtual Any get_field_const_void_ptr(const void        *obj,
                                                 const std::string &name) const = 0;

            /// Set field value (type-erased via void*)
            virtual void set_field_void_ptr(void *obj, const std::string &name,
                                            Any value) const = 0;

            // ================================================================
            // Type-erased property access (virtual fields via getter/setter)
            // ================================================================

            /// Check if a property exists
            virtual bool has_property(const std::string &name) const = 0;

            /// Get list of all property names
            virtual std::vector<std::string> get_properties() const = 0;

            /// Get property metadata by name
            virtual PropertyMeta get_property_info(const std::string &name) const = 0;

            /// Get property type
            virtual std::type_index get_property_type(const std::string &name) const = 0;

            /// Get property value (type-erased via void*)
            virtual Any get_property_void_ptr(void *obj, const std::string &name) const = 0;

            /// Get property value from const object
            virtual Any get_property_const_void_ptr(const void        *obj,
                                                    const std::string &name) const = 0;

            /// Set property value (type-erased via void*)
            virtual void set_property_void_ptr(void *obj, const std::string &name,
                                               Any value) const = 0;

            // ================================================================
            // Type-erased method access
            // ================================================================

            /// Check if a method exists
            virtual bool has_method(const std::string &name) const = 0;

            /// Invoke method on mutable object
            virtual Any invoke_method_void_ptr(void *obj, const std::string &name,
                                               std::vector<Any> args) const = 0;

            /// Invoke method on const object
            virtual Any invoke_const_method_void_ptr(const void *obj, const std::string &name,
                                                     std::vector<Any> args) const = 0;

            /// Get list of all method names
            virtual std::vector<std::string> get_methods() const = 0;

            /// Get method arity (number of parameters)
            virtual size_t get_method_arity(const std::string &name) const = 0;

            /// Get method parameter types
            virtual std::vector<std::type_index>
            get_method_arg_types(const std::string &name) const = 0;

            /// Get method return type
            virtual std::type_index get_method_return_type(const std::string &name) const = 0;
        };

        /**
         * @brief Concrete implementation of MetadataHolder for a specific class type
         */
        template <typename Class> struct MetadataHolderImpl : MetadataHolder {
            ClassMetadata<Class> metadata;

            explicit MetadataHolderImpl(std::string name) : metadata(std::move(name)) {}

            std::string get_name() const override { return metadata.name(); }

            const InheritanceInfo &get_inheritance() const override {
                return metadata.inheritance();
            }

            // ================================================================
            // Constructor access implementation
            // ================================================================

            std::vector<ConstructorMeta> get_constructors() const override {
                std::vector<ConstructorMeta> result;
                const auto                  &infos = metadata.constructor_infos();
                result.reserve(infos.size());
                for (const auto &info : infos) {
                    ConstructorMeta meta;
                    meta.param_types = info.param_types;
                    meta.arity       = info.arity;
                    result.push_back(meta);
                }
                return result;
            }

            std::string get_cpp_type_name() const override {
                return demangle(typeid(Class).name());
            }

            std::string get_base_class() const override {
                const auto &inheritance = metadata.inheritance();
                if (!inheritance.base_classes.empty()) {
                    return inheritance.base_classes[0].name;
                }
                return "";
            }

            MethodMeta get_method_info(const std::string &name) const override {
                MethodMeta meta;
                meta.param_types = metadata.get_method_arg_types(name);
                meta.return_type = metadata.get_method_return_type(name);
                meta.arity       = metadata.get_method_arity(name);
                meta.is_const    = false;
                return meta;
            }

            // ================================================================
            // Field access implementation (actual C++ member fields)
            // ================================================================

            bool has_field(const std::string &name) const override {
                const auto &fields = metadata.fields();
                return std::find(fields.begin(), fields.end(), name) != fields.end();
            }

            std::vector<std::string> get_fields() const override { return metadata.fields(); }

            std::type_index get_field_type(const std::string &name) const override {
                return metadata.get_field_type(name);
            }

            Any get_field_void_ptr(void *obj, const std::string &name) const override {
                Class *typed_obj = static_cast<Class *>(obj);
                return metadata.get_field(*typed_obj, name);
            }

            Any get_field_const_void_ptr(const void *obj, const std::string &name) const override {
                Class *typed_obj = const_cast<Class *>(static_cast<const Class *>(obj));
                return metadata.get_field(*typed_obj, name);
            }

            void set_field_void_ptr(void *obj, const std::string &name, Any value) const override {
                Class *typed_obj = static_cast<Class *>(obj);
                metadata.set_field(*typed_obj, name, std::move(value));
            }

            // ================================================================
            // Property access implementation (virtual fields via getter/setter)
            // ================================================================

            bool has_property(const std::string &name) const override {
                return metadata.is_property(name);
            }

            std::vector<std::string> get_properties() const override {
                return metadata.properties();
            }

            PropertyMeta get_property_info(const std::string &name) const override {
                const auto &info = metadata.get_property_info(name);
                PropertyMeta meta;
                meta.name        = info.name;
                meta.getter_name = info.getter_name;
                meta.setter_name = info.setter_name;
                meta.value_type  = info.value_type;
                meta.is_readonly = info.is_readonly;
                meta.is_writeonly = info.is_writeonly;
                return meta;
            }

            std::type_index get_property_type(const std::string &name) const override {
                return metadata.get_property_type(name);
            }

            Any get_property_void_ptr(void *obj, const std::string &name) const override {
                Class *typed_obj = static_cast<Class *>(obj);
                return metadata.get_property(*typed_obj, name);
            }

            Any get_property_const_void_ptr(const void *obj, const std::string &name) const override {
                Class *typed_obj = const_cast<Class *>(static_cast<const Class *>(obj));
                return metadata.get_property(*typed_obj, name);
            }

            void set_property_void_ptr(void *obj, const std::string &name, Any value) const override {
                Class *typed_obj = static_cast<Class *>(obj);
                metadata.set_property(*typed_obj, name, std::move(value));
            }

            // ================================================================
            // Method access implementation
            // ================================================================

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

            std::type_index get_method_return_type(const std::string &name) const override {
                return metadata.get_method_return_type(name);
            }
        };

    private:
        std::unordered_map<std::string, std::unique_ptr<MetadataHolder>> classes_;
        std::unordered_map<const std::type_info *, std::string>          type_to_name_;

        Registry() = default;

        Registry(const Registry &)            = delete;
        Registry &operator=(const Registry &) = delete;

    public:
        static Registry                                &instance();
        template <typename Class> ClassMetadata<Class> &register_class(const std::string &name);
        template <typename Class> ClassMetadata<Class> &get();
        template <typename Class> const ClassMetadata<Class> &get() const;

        /// Get metadata holder by class name (non-const version for binding generators)
        MetadataHolder *get_by_name(const std::string &name);

        /// Get metadata holder by class name (const version)
        const MetadataHolder *get_by_name(const std::string &name) const;

        std::string                    get_class_name(const std::type_info &type) const;
        std::vector<std::string>       list_classes() const;
        bool                           has_class(const std::string &name) const;
        template <typename Class> bool has_class() const;
        size_t                         size() const;
        void                           clear();
    };

} // namespace rosetta::core

inline rosetta::core::Registry &rosetta_registry() {
    return rosetta::core::Registry::instance();
}

#include "inline/registry.hxx"
