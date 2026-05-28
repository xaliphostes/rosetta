// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED
// Prototype: per-type value formatter (JSON-ish) driven by C++26 reflection.
//
// One generic template, dispatched at compile time:
//   - bool              -> true / false
//   - arithmetic        -> number
//   - enum              -> enumerator name      (std::meta::enumerators_of)
//   - string-like       -> quoted, JSON-escaped
//   - range             -> [ ... ]
//   - class             -> { "field": value }   (std::meta::nonstatic_data_members_of)
//
// Order of the if-constexpr branches matters: bool before arithmetic,
// string before range, enum before class.

#include <experimental/meta>
#include <format>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace rosetta::ser {

    inline void emit_string(std::string_view s, std::string &out) {
        out += '"';
        for (char c : s) {
            switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\t': out += "\\t";  break;
            case '\r': out += "\\r";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                    out += std::format("\\u{:04x}", static_cast<unsigned>(c));
                else
                    out += c;
            }
        }
        out += '"';
    }

    template <typename T>
    concept string_like = std::is_convertible_v<const T &, std::string_view>;

    template <typename T> void serialize_value(const T &v, std::string &out);

    // Reflection-driven helpers are split out and constrained so the
    // `template for` over enumerators_of / nonstatic_data_members_of
    // only ever instantiates for the right kind of type.
    template <typename E>
        requires std::is_enum_v<E>
    constexpr std::string_view enum_name(E v) {
        template for (constexpr auto e :
                      std::define_static_array(std::meta::enumerators_of(^^E))) {
            if (v == [:e:])
                return std::meta::identifier_of(e);
        }
        return {};
    }

    template <typename C>
        requires std::is_class_v<C>
    void serialize_class(const C &v, std::string &out) {
        constexpr auto ctx = std::meta::access_context::current();
        out += '{';
        bool first = true;
        template for (constexpr auto m : std::define_static_array(
                          std::meta::nonstatic_data_members_of(^^C, ctx))) {
            if (!first) out += ',';
            first = false;
            emit_string(std::meta::identifier_of(m), out);
            out += ':';
            serialize_value(v.[:m:], out);
        }
        out += '}';
    }

    template <typename T> void serialize_value(const T &v, std::string &out) {
        using U = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<U, bool>) {
            out += v ? "true" : "false";
        }
        else if constexpr (std::is_arithmetic_v<U>) {
            out += std::format("{}", v);
        }
        else if constexpr (std::is_enum_v<U>) {
            auto name = enum_name(v);
            if (!name.empty())
                emit_string(name, out);
            else
                out += std::format("{}", static_cast<std::underlying_type_t<U>>(v));
        }
        else if constexpr (string_like<U>) {
            emit_string(std::string_view{v}, out);
        }
        else if constexpr (std::ranges::range<U>) {
            out += '[';
            bool first = true;
            for (auto const &elem : v) {
                if (!first) out += ',';
                first = false;
                serialize_value(elem, out);
            }
            out += ']';
        }
        else if constexpr (std::is_class_v<U>) {
            serialize_class(v, out);
        }
        else {
            static_assert(sizeof(U) == 0, "rosetta::ser: no serializer for this type");
        }
    }

    template <typename T> std::string to_string(const T &v) {
        std::string out;
        serialize_value(v, out);
        return out;
    }

} // namespace rosetta::ser

// =================== demo ===================

enum class Color { Red, Green, Blue };

struct Address {
    std::string street;
    int         number;
};

struct Person {
    std::string              name;
    int                      age;
    bool                     active;
    Color                    favorite_color;
    Address                  home;
    std::vector<std::string> tags;
};

int main() {
    Person p{
        .name           = "Alice",
        .age            = 30,
        .active         = true,
        .favorite_color = Color::Green,
        .home           = {"Rue Pasteur", 42},
        .tags           = {"admin", "ops"},
    };

    std::println("{}", rosetta::ser::to_string(p));

    // Standalone values exercise each branch:
    // std::println("{}", rosetta::ser::to_string(42));
    // std::println("{}", rosetta::ser::to_string(3.14));
    // std::println("{}", rosetta::ser::to_string(true));
    // std::println("{}", rosetta::ser::to_string(Color::Blue));
    // std::println("{}", rosetta::ser::to_string(std::vector<int>{1, 2, 3}));
    // std::println("{}", rosetta::ser::to_string(std::string{"a \"quoted\" \n line"}));

    return 0;
}
