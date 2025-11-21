// ============================================================================
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
        static VirtualMethodRegistry &instance();
        template <typename Class>
        void register_virtual_method(const std::string &method_name, const std::string &signature,
                                     bool is_pure = false);
        template <typename Class> const VirtualTableInfo *get_vtable() const;
        template <typename Class> VirtualTableInfo       *get_vtable();
        template <typename Class> bool                    has_pure_virtual_methods() const;
        template <typename Class>
        void register_method_thunk(const std::string &method_name, std::function<void *()> thunk);
        void clear();
    };

} // namespace rosetta::core

#include "inline/virtual_method_registry.hxx"
