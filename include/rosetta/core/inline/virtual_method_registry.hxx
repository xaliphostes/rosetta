namespace rosetta::core {

    inline VirtualMethodRegistry &VirtualMethodRegistry::instance() {
        static VirtualMethodRegistry registry;
        return registry;
    }

    template <typename Class>
    inline void VirtualMethodRegistry::register_virtual_method(const std::string &method_name,
                                                                const std::string &signature,
                                                                bool               is_pure) {
        std::string class_name = typeid(Class).name();
        auto       &vtable     = class_vtables_[class_name].vtable;
        vtable.add_virtual_method(method_name, signature, is_pure);
    }

    template <typename Class>
    inline const VirtualTableInfo *VirtualMethodRegistry::get_vtable() const {
        std::string class_name = typeid(Class).name();
        auto        it         = class_vtables_.find(class_name);
        return it != class_vtables_.end() ? &it->second.vtable : nullptr;
    }

    template <typename Class> inline VirtualTableInfo *VirtualMethodRegistry::get_vtable() {
        std::string class_name = typeid(Class).name();
        auto        it         = class_vtables_.find(class_name);
        return it != class_vtables_.end() ? &it->second.vtable : nullptr;
    }

    template <typename Class> inline bool VirtualMethodRegistry::has_pure_virtual_methods() const {
        auto *vtable = get_vtable<Class>();
        if (!vtable) {
            return false;
        }
        return vtable->has_pure_virtual_methods();
    }

    template <typename Class>
    inline void VirtualMethodRegistry::register_method_thunk(const std::string      &method_name,
                                                             std::function<void *()> thunk) {
        std::string class_name                                = typeid(Class).name();
        class_vtables_[class_name].method_thunks[method_name] = std::move(thunk);
    }

    inline void VirtualMethodRegistry::clear() {
        class_vtables_.clear();
    }

} // namespace rosetta::core