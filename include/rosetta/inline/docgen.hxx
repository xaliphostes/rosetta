// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

namespace rosetta {

    namespace docgen_detail {

        // display_string_of() yields the canonical (mangled-looking) spelling
        // of standard types. This rewrites the handful of common offenders so
        // human-facing docs say `std::string` instead of
        // `basic_string<char, char_traits<char>, allocator<char>>`.
        inline std::string prettify(std::string s) {
            struct Rule {
                const char *from;
                const char *to;
            };
            static constexpr Rule rules[] = {
                {"basic_string<char, char_traits<char>, allocator<char>>", "std::string"},
                {"basic_string_view<char, char_traits<char>>", "std::string_view"},
            };
            for (const auto &r : rules) {
                std::string::size_type pos  = 0;
                const std::string      from = r.from;
                const std::string      to   = r.to;
                while ((pos = s.find(from, pos)) != std::string::npos) {
                    s.replace(pos, from.size(), to);
                    pos += to.size();
                }
            }
            return s;
        }

    } // namespace docgen_detail

    inline void MarkdownDoc::ensure_fields_header() {
        if (fields_header_emitted)
            return;
        out << "## Fields\n\n"
            << "| Name | Type | Description |\n"
            << "|------|------|-------------|\n";
        fields_header_emitted = true;
    }

    inline void MarkdownDoc::ensure_methods_header() {
        if (methods_header_emitted)
            return;
        out << (fields_header_emitted ? "\n" : "") << "## Methods\n\n";
        methods_header_emitted = true;
    }

    template <std::meta::info Fld, auto... Anns> void inline MarkdownDoc::field(const char *name) {
        constexpr const char *type_str =
            std::define_static_string(std::meta::display_string_of(std::meta::type_of(Fld)));
        constexpr auto dann    = ann::get_or<doc>(doc{""}, Anns...);
        constexpr bool ro      = ann::has<readonly>(Anns...);
        constexpr bool has_rng = ann::has<range>(Anns...);
        constexpr auto rng     = ann::get_or<range>(range{0, 0}, Anns...);
        constexpr bool has_cb  = ann::has<combobox>(Anns...);
        constexpr auto cb      = ann::get_or<combobox>(combobox{}, Anns...);

        ensure_fields_header();

        std::string desc = dann.text;
        if constexpr (has_rng) {
            std::ostringstream tag;
            tag << "(range: " << rng.min << ".." << rng.max << ")";
            if (!desc.empty())
                desc += " ";
            desc += tag.str();
        }
        if constexpr (has_cb) {
            std::ostringstream tag;
            tag << "(choices: ";
            for (std::size_t i = 0; i < cb.count; ++i) {
                if (i)
                    tag << ", ";
                tag << cb.choices[i];
            }
            tag << ")";
            if (!desc.empty())
                desc += " ";
            desc += tag.str();
        }
        if constexpr (ro) {
            if (!desc.empty())
                desc += " ";
            desc += "_(readonly)_";
        }

        out << "| `" << name << "` | `" << docgen_detail::prettify(type_str) << "` | " << desc
            << " |\n";
    }

    template <std::meta::info Fn, auto... Anns>
    inline void MarkdownDoc::method_instance(const char *name) {
        emit_method<Fn, Anns...>(name, /*is_static*/ false);
    }

    template <std::meta::info Fn, auto... Anns>
    inline void MarkdownDoc::method_static(const char *name) {
        emit_method<Fn, Anns...>(name, /*is_static*/ true);
    }

    // private:
    template <std::meta::info Fn, auto... Anns>
    inline void MarkdownDoc::emit_method(const char *name, bool is_static) {
        constexpr auto        dann = ann::get_or<doc>(doc{""}, Anns...);
        constexpr const char *ret_str =
            std::define_static_string(std::meta::display_string_of(std::meta::return_type_of(Fn)));

        ensure_methods_header();

        out << "### `";
        if (is_static)
            out << "static ";
        out << name << "(";

        bool first = true;
        template for (constexpr auto p : std::define_static_array(std::meta::parameters_of(Fn))) {
            constexpr const char *pname = std::define_static_string(std::meta::identifier_of(p));
            constexpr const char *ptype =
                std::define_static_string(std::meta::display_string_of(std::meta::type_of(p)));
            if (!first)
                out << ", ";
            first = false;
            if (pname[0] != '\0')
                out << pname << ": ";
            out << docgen_detail::prettify(ptype);
        }
        out << ") → " << docgen_detail::prettify(ret_str) << "`\n\n";

        if (dann.text[0] != '\0') {
            out << dann.text << "\n\n";
        }
    }

    // -----------------------------------------------------------------------
    
    // Explicit type name (use this when display_string_of(^^T) is ugly,
    // e.g. for template instantiations like std::vector<int>).
    template <typename T> inline std::string generate_markdown(const char *type_name) {
        MarkdownDoc d;
        d.out << "# " << type_name << "\n\n";
        walk<T>(d);
        return d.out.str();
    }

    // Auto-derived heading via std::meta::identifier_of(^^T).
    template <typename T> inline std::string generate_markdown() {
        constexpr const char *type_name = std::define_static_string(std::meta::identifier_of(^^T));
        return generate_markdown<T>(type_name);
    }

} // namespace rosetta
