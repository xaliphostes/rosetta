namespace rosetta::core {

    inline BaseClassInfo::BaseClassInfo(std::string n, const std::type_info *t,
                                        InheritanceType itype, AccessSpecifier acc, size_t off)
        : name(std::move(n)), type(t), inheritance_type(itype), access(acc), offset(off) {
    }

    inline void InheritanceInfo::add_base(const std::string &name, const std::type_info *type,
                                          InheritanceType itype, AccessSpecifier access,
                                          size_t offset) {
        BaseClassInfo info(name, type, itype, access, offset);

        if (itype == InheritanceType::Virtual) {
            virtual_bases.push_back(info);
        } else {
            base_classes.push_back(info);
        }
    }

    inline bool InheritanceInfo::has_base(const std::type_info &type) const {
        for (const auto &base : base_classes) {
            if (base.type && *base.type == type) {
                return true;
            }
        }
        for (const auto &base : virtual_bases) {
            if (base.type && *base.type == type) {
                return true;
            }
        }
        return false;
    }

    inline const BaseClassInfo *InheritanceInfo::get_base(const std::type_info &type) const {
        for (const auto &base : base_classes) {
            if (base.type && *base.type == type) {
                return &base;
            }
        }
        for (const auto &base : virtual_bases) {
            if (base.type && *base.type == type) {
                return &base;
            }
        }
        return nullptr;
    }

    inline size_t InheritanceInfo::total_base_count() const {
        return base_classes.size() + virtual_bases.size();
    }

} // namespace rosetta::core