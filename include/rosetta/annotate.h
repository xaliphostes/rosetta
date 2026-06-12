// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// =============================================================================
// rosetta/annotate.h
//
// Out-of-line annotations: keep your headers clean and describe the same
// metadata (doc / range / readonly / combobox) in a side-car JSON file that is
// baked into the program at compile time.
//
// The JSON is parsed at *compile time* and merged with whatever inline P3394
// annotations the member already carries -- so the two sources concatenate, and
// a class with no inline annotations and no side-car is simply un-annotated.
// Nothing here turns JSON into real P3394 annotations (that needs token
// injection, P3294, which clang-p2996 lacks); instead the parsed entries join
// the annotation pack at the single choke point in rosetta::walk().
//
// JSON schema (object keyed by member identifier):
//
//   {
//     "title": { "doc": "The widget title" },
//     "count": { "doc": "Visible items", "range": [0, 100] },
//     "id":    { "readonly": true },
//     "mode":  { "combobox": ["fast", "slow"] }
//   }
//
// Wiring: add an "annotations" field to the class entry in manifest.json;
// rosetta_gen bakes an ann_json_source<T> specialization into the generated
// bindings.h. The user's header and source stay free of annotation wiring.
// See docs/OUT_OF_LINE_ANNOTATIONS.md. (This header only provides the
// customization point, the parser, and the walk()-time merge; it never needs
// #embed -- the bytes are baked at generation time.)
//
// Note: the minimal parser below takes string values literally (no escape
// processing) and parses numbers as plain decimals -- enough for the annotation
// schema, not a general-purpose JSON library.
// =============================================================================

#pragma once

#include <array>
#include <cstddef>
#include <experimental/meta>
#include <rosetta/annotations.h>
#include <string>
#include <string_view>
#include <vector>

namespace rosetta {

    namespace detail {
        // Backing storage for a type's baked JSON bytes. rosetta_gen specializes
        // this per type with the side-car contents (as a std::to_array<char>).
        template <class T> inline constexpr std::array<char, 1> ann_storage = {'\0'};
    } // namespace detail

    // Customization point. Specialized (per type) by the generated bindings.h to
    // view the baked JSON bytes. The primary template is the "no side-car" case:
    // an empty source parses to an empty table, i.e. no annotations.
    template <class T> constexpr std::string_view ann_json_source = std::string_view{};

    namespace ann_json {

        // ---- parsed representation (all constexpr-friendly, no allocation) ----

        struct choice_list {
            std::string_view items[combobox::MAX]{};
            std::size_t      count = 0;
        };

        struct member_ann {
            std::string_view name{};
            bool             has_doc      = false;
            std::string_view doc{};
            bool             has_range    = false;
            double           rmin         = 0;
            double           rmax         = 0;
            bool             has_readonly = false;
            bool             readonly_on  = false;
            bool             has_combobox = false;
            choice_list      combo{};
        };

        inline constexpr std::size_t MAX_MEMBERS = 256;

        struct table {
            member_ann  items[MAX_MEMBERS]{};
            std::size_t count = 0;
            bool        ok    = true; // false on malformed input
        };

        // ---- minimal recursive-descent parser over a string_view ----

        consteval double to_double(std::string_view t) {
            std::size_t i    = 0;
            double      sign = 1;
            if (i < t.size() && (t[i] == '+' || t[i] == '-')) {
                if (t[i] == '-') sign = -1;
                ++i;
            }
            double v = 0;
            for (; i < t.size() && t[i] >= '0' && t[i] <= '9'; ++i) v = v * 10 + (t[i] - '0');
            if (i < t.size() && t[i] == '.') {
                ++i;
                double scale = 0.1;
                for (; i < t.size() && t[i] >= '0' && t[i] <= '9'; ++i) {
                    v += (t[i] - '0') * scale;
                    scale *= 0.1;
                }
            }
            return sign * v;
        }

        struct parser {
            std::string_view s;
            std::size_t      i = 0;

            consteval void ws() {
                while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
                    ++i;
            }
            consteval bool eat(char c) {
                ws();
                if (i < s.size() && s[i] == c) {
                    ++i;
                    return true;
                }
                return false;
            }
            consteval bool keyword(std::string_view kw) {
                ws();
                if (s.substr(i, kw.size()) == kw) {
                    i += kw.size();
                    return true;
                }
                return false;
            }
            // String value -- returned verbatim (escapes are not unescaped, but
            // an escaped quote does not terminate the string).
            consteval std::string_view str() {
                ws();
                if (i >= s.size() || s[i] != '"') return {};
                ++i;
                std::size_t start = i;
                while (i < s.size() && s[i] != '"') {
                    if (s[i] == '\\' && i + 1 < s.size()) ++i;
                    ++i;
                }
                std::string_view r = s.substr(start, i - start);
                if (i < s.size()) ++i; // closing quote
                return r;
            }
            consteval double number() {
                ws();
                std::size_t start = i;
                if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
                while (i < s.size() && ((s[i] >= '0' && s[i] <= '9') || s[i] == '.')) ++i;
                return to_double(s.substr(start, i - start));
            }
            // Skip one value of an unrecognized key (string / number / bool /
            // null / nested array / object), so unknown keys don't derail parse.
            consteval void skip_value() {
                ws();
                if (i >= s.size()) return;
                char c = s[i];
                if (c == '"') {
                    str();
                } else if (c == '[' || c == '{') {
                    char open = c, close = (c == '[') ? ']' : '}';
                    int  depth = 0;
                    for (; i < s.size(); ++i) {
                        if (s[i] == '"') {
                            str();
                            --i; // str() advanced past; loop will ++i
                            continue;
                        }
                        if (s[i] == open) ++depth;
                        else if (s[i] == close && --depth == 0) {
                            ++i;
                            break;
                        }
                    }
                } else if (keyword("true") || keyword("false") || keyword("null")) {
                    // consumed
                } else {
                    number();
                }
            }
        };

        consteval table parse(std::string_view src) {
            table  t;
            parser p{src};
            p.ws();
            if (src.empty() || p.i >= src.size()) return t; // no file -> empty table
            if (!p.eat('{')) {
                t.ok = false;
                return t;
            }
            if (p.eat('}')) return t; // {}
            do {
                member_ann m;
                m.name = p.str();
                if (m.name.empty() || !p.eat(':') || !p.eat('{')) {
                    t.ok = false;
                    break;
                }
                if (!p.eat('}')) {
                    do {
                        std::string_view k = p.str();
                        if (!p.eat(':')) {
                            t.ok = false;
                            break;
                        }
                        if (k == "doc") {
                            m.has_doc = true;
                            m.doc     = p.str();
                        } else if (k == "range") {
                            m.has_range = true;
                            p.eat('[');
                            m.rmin = p.number();
                            p.eat(',');
                            m.rmax = p.number();
                            p.eat(']');
                        } else if (k == "readonly") {
                            m.has_readonly = true;
                            if (p.keyword("true")) m.readonly_on = true;
                            else if (p.keyword("false")) m.readonly_on = false;
                            else m.readonly_on = (p.number() != 0);
                        } else if (k == "combobox") {
                            m.has_combobox = true;
                            p.eat('[');
                            if (!p.eat(']')) {
                                do {
                                    std::string_view c = p.str();
                                    if (m.combo.count < combobox::MAX)
                                        m.combo.items[m.combo.count++] = c;
                                } while (p.eat(','));
                                p.eat(']');
                            }
                        } else {
                            p.skip_value();
                        }
                    } while (p.eat(','));
                    p.eat('}');
                }
                if (t.count < MAX_MEMBERS) t.items[t.count++] = m;
            } while (p.eat(','));
            p.eat('}');
            return t;
        }

    } // namespace ann_json

    namespace detail {

        // JSON-sourced annotations for one member name, as reflections of
        // constants -- directly spliceable as NTTPs, same shape as the entries
        // std::meta::constant_of() yields for inline annotations.
        template <class T>
        consteval std::vector<std::meta::info> json_annotations_for(std::string_view member) {
            std::vector<std::meta::info> out;
            auto                         t = ann_json::parse(rosetta::ann_json_source<T>);
            for (std::size_t k = 0; k < t.count; ++k) {
                const auto &m = t.items[k];
                if (m.name != member) continue;
                if (m.has_doc)
                    out.push_back(
                        std::meta::reflect_constant(rosetta::doc{std::define_static_string(m.doc)}));
                if (m.has_range)
                    out.push_back(std::meta::reflect_constant(rosetta::range{m.rmin, m.rmax}));
                if (m.has_readonly && m.readonly_on)
                    out.push_back(std::meta::reflect_constant(rosetta::readonly{}));
                if (m.has_combobox) {
                    rosetta::combobox cb{};
                    cb.count = m.combo.count;
                    for (std::size_t j = 0; j < m.combo.count; ++j)
                        cb.choices[j] = std::define_static_string(m.combo.items[j]);
                    out.push_back(std::meta::reflect_constant(cb));
                }
            }
            return out;
        }

        // Inline annotations (normalized to constant reflections) ++ JSON ones.
        // Inline come first, so when a visitor reads "the first / last of kind A"
        // the precedence is well-defined; doc-style kinds that a backend chooses
        // to render cumulatively simply see both.
        template <class T>
        consteval std::vector<std::meta::info> merged_annotations(std::meta::info member) {
            std::vector<std::meta::info> v;
            for (auto a : std::meta::annotations_of(member)) v.push_back(std::meta::constant_of(a));
            for (auto e : json_annotations_for<T>(std::meta::identifier_of(member))) v.push_back(e);
            return v;
        }

        // Compile-time guard: every JSON key must name a real member of T, so a
        // renamed/typo'd field fails the build instead of silently losing data.
        // Returns "" when valid, otherwise a message naming the offending key
        // (used as a P2741 constexpr static_assert message in bindings.h).
        template <class T> consteval std::string ann_keys_error() {
            constexpr auto ctx = std::meta::access_context::current();
            auto           t   = ann_json::parse(rosetta::ann_json_source<T>);
            if (!t.ok)
                return std::string("rosetta out-of-line annotations: malformed JSON side-car");
            for (std::size_t k = 0; k < t.count; ++k) {
                bool found = false;
                for (auto m : std::meta::nonstatic_data_members_of(^^T, ctx))
                    if (std::meta::identifier_of(m) == t.items[k].name) {
                        found = true;
                        break;
                    }
                if (!found)
                    for (auto m : std::meta::members_of(^^T, ctx))
                        if (std::meta::is_function(m) && std::meta::has_identifier(m) &&
                            std::meta::identifier_of(m) == t.items[k].name) {
                            found = true;
                            break;
                        }
                if (!found)
                    return std::string("rosetta out-of-line annotations: JSON key \"") +
                           std::string(t.items[k].name) +
                           "\" does not name any member of the annotated type "
                           "(renamed/typo'd field?)";
            }
            return std::string{};
        }

    } // namespace detail

} // namespace rosetta
