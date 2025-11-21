// ============================================================================
// Informations sur les méthodes virtuelles et la vtable
// ============================================================================
#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace rosetta::core {

    struct VirtualMethodInfo {
        std::string name;
        std::string signature;
        bool        is_pure_virtual = false;
        bool        is_override     = false;
        bool        is_final        = false;
        size_t      vtable_index    = 0;

        VirtualMethodInfo() = default;

        VirtualMethodInfo(std::string n, std::string sig, bool pure = false);
    };

    struct VirtualTableInfo {
        std::vector<VirtualMethodInfo> methods;
        size_t                         vtable_ptr_offset = 0;

        void add_virtual_method(const std::string &name, const std::string &signature,
                                bool is_pure = false);
        const VirtualMethodInfo *find_method(const std::string &name) const;
        VirtualMethodInfo       *find_method(const std::string &name);
        bool                     has_pure_virtual_methods() const;
    };

} // namespace rosetta::core

#include "inline/virtual_method_info.hxx"
