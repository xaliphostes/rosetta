namespace rosetta::moc {

    // -------------------------------------------------------------------------
    // Reflection helpers.
    // -------------------------------------------------------------------------
    namespace detail {

        template <class Tag, class T>
        consteval std::meta::info find_tagged(std::string_view name) {
            constexpr auto ctx = std::meta::access_context::unchecked();
            for (auto m : std::meta::members_of(^^T, ctx)) {
                if (std::meta::annotation_of_type<Tag>(m).has_value() && std::meta::identifier_of(m) == name) {
                    return m;
                }
            }
            return {};
        }

        template <class T>
        consteval std::meta::info find_property_field(std::string_view name) {
            constexpr auto ctx = std::meta::access_context::unchecked();
            for (auto m : std::meta::nonstatic_data_members_of(^^T, ctx)) {
                auto pa = std::meta::annotation_of_type<property>(m);
                if (pa.has_value() && std::string_view{pa->name} == name) {
                    return m;
                }
            }
            return {};
        }

    } // namespace detail

    // -------------------------------------------------------------------------
    // connect<"sig","slot">(sender, receiver)
    //
    // Compile-time-checked: missing names static_assert, mismatched argument
    // types fail at the lambda body.
    // -------------------------------------------------------------------------
    template <fixed_string Sig, fixed_string Slot, class S, class R>
    inline void connect(S &sender, R &receiver) {
        constexpr auto sig_r  = detail::find_tagged<signal_tag, S>(Sig.view());
        constexpr auto slot_r = detail::find_tagged<slot_tag, R>(Slot.view());
        static_assert(sig_r != std::meta::info{}, "no [[=signal]]-tagged member with that name on sender");
        static_assert(slot_r != std::meta::info{}, "no [[=slot]]-tagged member with that name on receiver");

        (sender.[:sig_r:]).connect([&receiver](auto &&...args) {
            (receiver.[:slot_r:])(std::forward<decltype(args)>(args)...);
        });
    }

    // -------------------------------------------------------------------------
    // get<"prop">(obj): read access for a [[=property]]-annotated field.
    // -------------------------------------------------------------------------
    template <fixed_string Name, class T> inline auto const &get(T const &obj) {
        constexpr auto field_r = detail::find_property_field<T>(Name.view());
        static_assert(field_r != std::meta::info{}, "no [[=property]] with that handle on T");
        return obj.[:field_r:];
    }

    // -------------------------------------------------------------------------
    // set<"prop">(obj, value): write access. Equality-gated; if the property
    // declares a NOTIFY signal, fires it after a successful change.
    // -------------------------------------------------------------------------
    template <fixed_string Name, class T, class V>
    inline void set(T &obj, V &&value) {
        constexpr auto field_r = detail::find_property_field<T>(Name.view());
        static_assert(field_r != std::meta::info{}, "no [[=property]] with that handle on T");

        using F = [:std::meta::type_of(field_r):];
        F v(std::forward<V>(value));
        if (obj.[:field_r:] == v) {
            return;
        }
        obj.[:field_r:] = std::move(v);

        constexpr auto pa = std::meta::annotation_of_type<property>(field_r).value();
        constexpr auto notify_name = std::string_view{pa.notify};
        if constexpr (!notify_name.empty()) {
            constexpr auto sig_r = detail::find_tagged<signal_tag, T>(notify_name);
            static_assert(sig_r != std::meta::info{}, "NOTIFY signal named by property is not declared");
            (obj.[:sig_r:])(obj.[:field_r:]);
        }
    }

} // namespace rosetta::moc
