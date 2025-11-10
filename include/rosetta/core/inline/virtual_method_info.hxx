namespace rosetta::core {

    inline VirtualMethodInfo::VirtualMethodInfo(std::string n, std::string sig, bool pure)
        : name(std::move(n)), signature(std::move(sig)), is_pure_virtual(pure) {
    }
    inline void VirtualTableInfo::add_virtual_method(const std::string &name,
                                                     const std::string &signature,
                                                     bool               is_pure) {
        methods.emplace_back(name, signature, is_pure);
        methods.back().vtable_index = methods.size() - 1;
    }

    inline const VirtualMethodInfo *VirtualTableInfo::find_method(const std::string &name) const {
        for (const auto &method : methods) {
            if (method.name == name) {
                return &method;
            }
        }
        return nullptr;
    }

    inline VirtualMethodInfo *VirtualTableInfo::find_method(const std::string &name) {
        for (auto &method : methods) {
            if (method.name == name) {
                return &method;
            }
        }
        return nullptr;
    }

    inline bool VirtualTableInfo::has_pure_virtual_methods() const {
        for (const auto &method : methods) {
            if (method.is_pure_virtual) {
                return true;
            }
        }
        return false;
    }

} // namespace rosetta::core