// ============================================================================
// Type information system for JavaScript bindings
// Provides better type tracking than raw typeid().name()
// ============================================================================
#pragma once

#include <array>
#include <cmath>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace rosetta::generators {

    /**
     * @brief Type category for better type handling
     */
    enum class TypeCategory {
        Primitive, // int, double, float, bool
        String,    // std::string
        Object,    // User-defined classes
        Pointer,   // T*, std::shared_ptr<T>, std::unique_ptr<T>
        Container, // std::vector, std::array, std::map
        Optional,  // std::optional<T>
        Function,  // Function pointers, std::function
        Unknown    // Fallback
    };

    /**
     * @brief Comprehensive type information
     *
     * Provides detailed information about C++ types for JavaScript binding
     */
    struct TypeInfo {
        std::string     name;         // Human-readable name (e.g., "int", "Person")
        std::string     mangled_name; // typeid().name() output
        std::type_index type_index;   // For hash map lookups
        TypeCategory    category;     // Type category
        bool            is_const;     // Is const-qualified
        bool            is_reference; // Is reference type
        bool            is_pointer;   // Is pointer type
        size_t          size;         // sizeof(T)
        size_t          alignment;    // alignof(T)

        // For template types
        bool                  is_template;   // Is a template instantiation
        std::string           template_name; // Template name (e.g., "std::vector")
        std::vector<TypeInfo> template_args; // Template arguments

        TypeInfo()
            : type_index(typeid(void)), category(TypeCategory::Unknown), is_const(false),
              is_reference(false), is_pointer(false), size(0), alignment(0), is_template(false) {}

        /**
         * @brief Create TypeInfo from a C++ type
         */
        template <typename T> static TypeInfo create() {
            TypeInfo info;

            using RawType = std::remove_cv_t<std::remove_reference_t<T>>;

            info.type_index   = std::type_index(typeid(RawType));
            info.mangled_name = typeid(RawType).name();
            info.is_const     = std::is_const_v<std::remove_reference_t<T>>;
            info.is_reference = std::is_reference_v<T>;
            info.is_pointer   = std::is_pointer_v<RawType>;
            info.size         = sizeof(RawType);
            info.alignment    = alignof(RawType);

            // Determine category and name
            if constexpr (std::is_same_v<RawType, int> || std::is_same_v<RawType, long> ||
                          std::is_same_v<RawType, short> || std::is_same_v<RawType, char>) {
                info.category = TypeCategory::Primitive;
                info.name     = "int";
            } else if constexpr (std::is_same_v<RawType, unsigned int> ||
                                 std::is_same_v<RawType, unsigned long> ||
                                 std::is_same_v<RawType, unsigned short> ||
                                 std::is_same_v<RawType, unsigned char>) {
                info.category = TypeCategory::Primitive;
                info.name     = "uint";
            } else if constexpr (std::is_same_v<RawType, float>) {
                info.category = TypeCategory::Primitive;
                info.name     = "float";
            } else if constexpr (std::is_same_v<RawType, double>) {
                info.category = TypeCategory::Primitive;
                info.name     = "double";
            } else if constexpr (std::is_same_v<RawType, bool>) {
                info.category = TypeCategory::Primitive;
                info.name     = "bool";
            } else if constexpr (std::is_same_v<RawType, std::string>) {
                info.category = TypeCategory::String;
                info.name     = "string";
            } else if constexpr (is_vector<RawType>::value) {
                info.category      = TypeCategory::Container;
                info.is_template   = true;
                info.template_name = "std::vector";
                info.name          = "vector";
                // Extract element type info
                using ElementType = typename RawType::value_type;
                info.template_args.push_back(create<ElementType>());
            } else if constexpr (is_array<RawType>::value) {
                info.category      = TypeCategory::Container;
                info.is_template   = true;
                info.template_name = "std::array";
                info.name          = "array";
            } else if constexpr (is_map<RawType>::value) {
                info.category      = TypeCategory::Container;
                info.is_template   = true;
                info.template_name = "std::map";
                info.name          = "map";
            } else if constexpr (is_optional<RawType>::value) {
                info.category      = TypeCategory::Optional;
                info.is_template   = true;
                info.template_name = "std::optional";
                info.name          = "optional";
                // Extract value type info
                using ValueType = typename RawType::value_type;
                info.template_args.push_back(create<ValueType>());
            } else if constexpr (is_shared_ptr<RawType>::value) {
                info.category      = TypeCategory::Pointer;
                info.is_template   = true;
                info.template_name = "std::shared_ptr";
                info.name          = "shared_ptr";
            } else if constexpr (is_unique_ptr<RawType>::value) {
                info.category      = TypeCategory::Pointer;
                info.is_template   = true;
                info.template_name = "std::unique_ptr";
                info.name          = "unique_ptr";
            } else if constexpr (std::is_class_v<RawType>) {
                info.category = TypeCategory::Object;
                info.name = typeid(RawType).name(); // Will be overridden by registry if available
            } else {
                info.category = TypeCategory::Unknown;
                info.name     = typeid(RawType).name();
            }

            return info;
        }

        /**
         * @brief Get a full descriptive name
         */
        std::string full_name() const {
            std::string result = name;

            if (is_template && !template_args.empty()) {
                result += "<";
                for (size_t i = 0; i < template_args.size(); ++i) {
                    if (i > 0)
                        result += ", ";
                    result += template_args[i].full_name();
                }
                result += ">";
            }

            if (is_const)
                result = "const " + result;
            if (is_reference)
                result += "&";
            if (is_pointer)
                result += "*";

            return result;
        }

        /**
         * @brief Check if this is a numeric type
         */
        bool is_numeric() const {
            return category == TypeCategory::Primitive &&
                   (name == "int" || name == "uint" || name == "float" || name == "double");
        }

        /**
         * @brief Check if this is an integer type
         */
        bool is_integer() const {
            return category == TypeCategory::Primitive && (name == "int" || name == "uint");
        }

        /**
         * @brief Check if this is a floating point type
         */
        bool is_floating() const {
            return category == TypeCategory::Primitive && (name == "float" || name == "double");
        }

    private:
        // Type trait helpers for template detection
        template <typename T> struct is_vector : std::false_type {};

        template <typename T, typename A> struct is_vector<std::vector<T, A>> : std::true_type {};

        template <typename T> struct is_array : std::false_type {};

        template <typename T, size_t N> struct is_array<std::array<T, N>> : std::true_type {};

        template <typename T> struct is_map : std::false_type {};

        template <typename K, typename V, typename C, typename A>
        struct is_map<std::map<K, V, C, A>> : std::true_type {};

        template <typename K, typename V, typename H, typename E, typename A>
        struct is_map<std::unordered_map<K, V, H, E, A>> : std::true_type {};

        template <typename T> struct is_optional : std::false_type {};

        template <typename T> struct is_optional<std::optional<T>> : std::true_type {};

        template <typename T> struct is_shared_ptr : std::false_type {};

        template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

        template <typename T> struct is_unique_ptr : std::false_type {};

        template <typename T> struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};
    };

    /**
     * @brief Type registry for managing TypeInfo instances
     */
    class TypeRegistry {
        std::unordered_map<std::type_index, TypeInfo>    types_;
        std::unordered_map<std::string, std::type_index> name_to_index_;

        TypeRegistry() = default;

    public:
        static TypeRegistry &instance() {
            static TypeRegistry registry;
            return registry;
        }

        /**
         * @brief Register a type with custom name
         */
        template <typename T> void register_type(const std::string &custom_name = "") {
            TypeInfo info = TypeInfo::create<T>();

            if (!custom_name.empty()) {
                info.name = custom_name;
            }

            types_.emplace(info.type_index, info);
            name_to_index_.emplace(info.name, info.type_index);
        }

        /**
         * @brief Get TypeInfo for a C++ type
         */
        template <typename T> const TypeInfo &get() const {
            using RawType = std::remove_cv_t<std::remove_reference_t<T>>;
            std::type_index idx(typeid(RawType));

            auto it = types_.find(idx);
            if (it != types_.end()) {
                return it->second;
            }

            // Return a default-constructed TypeInfo if not found
            static TypeInfo default_info = TypeInfo::create<T>();
            return default_info;
        }

        /**
         * @brief Get TypeInfo by type_index
         */
        const TypeInfo *get(std::type_index idx) const {
            auto it = types_.find(idx);
            return it != types_.end() ? &it->second : nullptr;
        }

        /**
         * @brief Get TypeInfo by name
         */
        const TypeInfo *get_by_name(const std::string &name) const {
            auto it = name_to_index_.find(name);
            if (it != name_to_index_.end()) {
                auto type_it = types_.find(it->second);
                if (type_it != types_.end()) {
                    return &type_it->second;
                }
            }
            return nullptr;
        }

        /**
         * @brief Check if a type is registered
         */
        template <typename T> bool has_type() const {
            using RawType = std::remove_cv_t<std::remove_reference_t<T>>;
            return types_.find(std::type_index(typeid(RawType))) != types_.end();
        }

        /**
         * @brief Get all registered type names
         */
        std::vector<std::string> list_types() const {
            std::vector<std::string> names;
            names.reserve(types_.size());
            for (const auto &[idx, info] : types_) {
                names.push_back(info.name);
            }
            return names;
        }

        /**
         * @brief Clear all registered types
         */
        void clear() {
            types_.clear();
            name_to_index_.clear();
        }
    };

} // namespace rosetta::generators