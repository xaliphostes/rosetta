#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#endif

namespace rosetta::core {

    /**
     * @brief Demangle a C++ type name
     * @param name Mangled name from typeid().name()
     * @return Demangled human-readable name
     */
    inline std::string demangle(const char *name) {
#ifdef __GNUG__
        // GCC/Clang demangling
        int                                     status = -1;
        std::unique_ptr<char, void (*)(void *)> result{
            abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};
        return (status == 0) ? result.get() : name;
#else
        // MSVC doesn't mangle names the same way
        // typeid().name() is already somewhat readable on MSVC
        return name;
#endif
    }

    /**
     * @brief Demangle a type_index
     */
    inline std::string demangle(const std::type_index &ti) {
        return demangle(ti.name());
    }

    /**
     * @brief Clean up type names for better readability
     * @param type_name Demangled type name
     * @return Cleaned up type name
     */
    inline std::string cleanup_type_name(const std::string &type_name) {
        std::string result = type_name;

        // Phase 1: Fix common implementation-specific namespaces
        static const std::unordered_map<std::string, std::string> namespace_fixes = {
            {"std::__1::", "std::"},     // libc++ namespace
            {"std::__cxx11::", "std::"}, // libstdc++ C++11 ABI
        };

        for (const auto &[from, to] : namespace_fixes) {
            size_t pos = 0;
            while ((pos = result.find(from, pos)) != std::string::npos) {
                result.replace(pos, from.length(), to);
                pos += to.length();
            }
        }

        // Phase 2: Remove allocator template parameters
        // Pattern: ", std::allocator<T>" or ", allocator<T>"
        {
            size_t pos = 0;
            while ((pos = result.find(", std::allocator<", pos)) != std::string::npos) {
                // Find the matching closing bracket for allocator<...>
                int    bracket_count = 1;
                size_t search_pos    = pos + 17; // Length of ", std::allocator<"

                while (search_pos < result.length() && bracket_count > 0) {
                    if (result[search_pos] == '<') {
                        bracket_count++;
                    } else if (result[search_pos] == '>') {
                        bracket_count--;
                    }
                    search_pos++;
                }

                if (bracket_count == 0) {
                    // Remove from ", std::allocator<" to the matching ">"
                    result.erase(pos, search_pos - pos);
                } else {
                    pos++; // Move forward if we couldn't find matching bracket
                }
            }
        }

        // Also handle allocator without std:: prefix
        {
            size_t pos = 0;
            while ((pos = result.find(", allocator<", pos)) != std::string::npos) {
                int    bracket_count = 1;
                size_t search_pos    = pos + 12; // Length of ", allocator<"

                while (search_pos < result.length() && bracket_count > 0) {
                    if (result[search_pos] == '<') {
                        bracket_count++;
                    } else if (result[search_pos] == '>') {
                        bracket_count--;
                    }
                    search_pos++;
                }

                if (bracket_count == 0) {
                    result.erase(pos, search_pos - pos);
                } else {
                    pos++;
                }
            }
        }

        // Phase 3: Simplify std::basic_string to string
        {
            size_t pos = 0;
            while ((pos = result.find("std::basic_string<char", pos)) != std::string::npos) {
                // Find the end of this basic_string template
                int    bracket_count = 1;
                size_t search_pos    = pos + 22; // After "std::basic_string<char"

                while (search_pos < result.length() && bracket_count > 0) {
                    if (result[search_pos] == '<') {
                        bracket_count++;
                    } else if (result[search_pos] == '>') {
                        bracket_count--;
                    }
                    search_pos++;
                }

                if (bracket_count == 0) {
                    result.replace(pos, search_pos - pos, "string");
                    pos += 6; // Length of "string"
                } else {
                    pos++;
                }
            }
        }

        // Phase 4: Remove std:: prefix (optional - do this last)
        {
            size_t pos = 0;
            while ((pos = result.find("std::", pos)) != std::string::npos) {
                result.erase(pos, 5); // Remove "std::"
                // Don't increment pos, check the same position again
            }
        }

        // Phase 5: Final cleanups
        {
            // Remove space before >
            size_t pos = 0;
            while ((pos = result.find(" >", pos)) != std::string::npos) {
                result.erase(pos, 1);
            }

            // Fix >> to > > for older C++ style (if desired, otherwise remove this)
            // Commented out as modern C++ accepts >>
            // pos = 0;
            // while ((pos = result.find(">>", pos)) != std::string::npos) {
            //     result.replace(pos, 2, "> >");
            //     pos += 3;
            // }
        }

        return result;
    }

    /**
     * @brief Get a human-readable type name
     * @tparam T Type to get name for
     * @return Readable type name
     */
    template <typename T> std::string get_readable_type_name() {
        std::string demangled = demangle(typeid(T).name());
        return cleanup_type_name(demangled);
    }

    /**
     * @brief Get a human-readable type name from type_index
     */
    inline std::string get_readable_type_name(const std::type_index &ti) {
        std::string demangled = demangle(ti);
        return cleanup_type_name(demangled);
    }

    // ------------------------------------------------------

    inline TypeNameRegistry &TypeNameRegistry::instance() {
        static TypeNameRegistry registry;
        return registry;
    }

    template <typename T> inline void TypeNameRegistry::register_name(const std::string &name) {
        type_names_[std::type_index(typeid(T))] = name;
    }

    inline std::string TypeNameRegistry::get_name(const std::type_index &ti) const {
        auto it = type_names_.find(ti);
        if (it != type_names_.end()) {
            return it->second;
        }
        return get_readable_type_name(ti);
    }

    /**
     * @brief Check if a type has a custom registered name
     */
    inline bool TypeNameRegistry::has_custom_name(const std::type_index &ti) const {
        return type_names_.find(ti) != type_names_.end();
    }

    inline void TypeNameRegistry::clear() {
        type_names_.clear();
    }

} // namespace rosetta::core