// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Definitions for <rosetta/walk.h>. Not a standalone header — it relies on the
// declarations and includes that walk.h sets up, and is included at its bottom.

namespace rosetta {

    namespace ann {

        template <typename A> consteval bool has(auto... anns) {
            return (std::same_as<std::remove_cvref_t<decltype(anns)>, A> || ...);
        }

        template <typename A> consteval A get_or(A fallback, auto... anns) {
            A result = fallback;
            (
                [&] {
                    if constexpr (std::same_as<std::remove_cvref_t<decltype(anns)>, A>) {
                        result = anns;
                    }
                }(),
                ...);
            return result;
        }

    } // namespace ann

    consteval bool is_exportable_member_function(std::meta::info fn) {
        return std::meta::is_function(fn) && !std::meta::is_constructor(fn) &&
               !std::meta::is_destructor(fn) && !std::meta::is_special_member_function(fn);
    }

    consteval bool is_exportable_constructor(std::meta::info fn) {
        return std::meta::is_constructor(fn) && !std::meta::is_copy_constructor(fn) &&
               !std::meta::is_move_constructor(fn) && !std::meta::is_constructor_template(fn) &&
               !std::meta::is_deleted(fn);
    }

    template <typename T, typename Visitor> void walk(Visitor &v) {
        constexpr auto ctx = std::meta::access_context::current();

        // -------- fields --------
        template for (constexpr auto fld :
                      std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
            constexpr auto name = std::define_static_string(std::meta::identifier_of(fld));
            // Inline P3394 annotations merged with any out-of-line ones from a
            // JSON side-car baked into ann_json_source<T> (see
            // <rosetta/annotate.h>). Entries are already constant reflections,
            // so they splice directly.
            constexpr auto anns = std::define_static_array(detail::merged_annotations<T>(fld));

            [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                v.template field<fld, ([:anns[Is]:])...>(name);
            }(std::make_index_sequence<anns.size()>{});
        }

        // -------- methods (instance + static) --------
        template for (constexpr auto fn :
                      std::define_static_array(std::meta::members_of(^^T, ctx))) {
            if constexpr (is_exportable_member_function(fn)) {
                constexpr auto name = std::define_static_string(std::meta::identifier_of(fn));
                constexpr auto anns = std::define_static_array(detail::merged_annotations<T>(fn));

                [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                    if constexpr (std::meta::is_static_member(fn)) {
                        v.template method_static<fn, ([:anns[Is]:])...>(name);
                    } else {
                        v.template method_instance<fn, ([:anns[Is]:])...>(name);
                    }
                }(std::make_index_sequence<anns.size()>{});
            }
        }

        // -------- constructors (optional visitor entry point) --------
        template for (constexpr auto ctor :
                      std::define_static_array(std::meta::members_of(^^T, ctx))) {
            if constexpr (is_exportable_constructor(ctor)) {
                constexpr auto anns = std::define_static_array(std::meta::annotations_of(ctor));

                [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                    if constexpr (requires {
                                      v.template constructor<ctor,
                                                             ([:std::meta::constant_of(anns[Is]):])...>();
                                  }) {
                        v.template constructor<ctor, ([:std::meta::constant_of(anns[Is]):])...>();
                    }
                }(std::make_index_sequence<anns.size()>{});
            }
        }
    }

} // namespace rosetta
