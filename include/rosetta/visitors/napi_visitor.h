// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// N-API backend: implements the rosetta::walk visitor concept.
//
// Provides:
//   - rosetta::to_napi / rosetta::from_napi — minimal type conversions
//   - rosetta::Wrap<T> — CRTP Napi::ObjectWrap holding T + per-Fld/per-Fn
//     member-function templates whose addresses feed InstanceAccessor /
//     InstanceMethod / StaticMethod as NTTPs
//   - rosetta::NapiVisitor<T> — three visitor methods that push descriptors
//   - rosetta::bind_napi<T>(env, name) — entry point: runs the walk and
//     calls Wrap<T>::DefineClass

#pragma once

#include <experimental/meta>
#include <napi.h>
#include <rosetta/walk.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace rosetta {

    // ---- Tiny type conversion helpers (extend as needed) ----

    template <typename T> Napi::Value to_napi(Napi::Env env, const T &v) {
        if constexpr (std::is_same_v<T, std::string>) {
            return Napi::String::New(env, v);
        } else if constexpr (std::is_same_v<T, bool>) {
            return Napi::Boolean::New(env, v);
        } else if constexpr (std::is_floating_point_v<T> || std::is_integral_v<T>) {
            return Napi::Number::New(env, static_cast<double>(v));
        } else {
            static_assert(sizeof(T) == 0, "to_napi: unsupported type");
        }
    }

    template <typename T> T from_napi(const Napi::Value &v) {
        if constexpr (std::is_same_v<T, std::string>) {
            return v.As<Napi::String>().Utf8Value();
        } else if constexpr (std::is_same_v<T, bool>) {
            return v.As<Napi::Boolean>().Value();
        } else if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(v.As<Napi::Number>().DoubleValue());
        } else if constexpr (std::is_integral_v<T>) {
            return static_cast<T>(v.As<Napi::Number>().Int64Value());
        } else {
            static_assert(sizeof(T) == 0, "from_napi: unsupported type");
        }
    }

    // ---- CRTP wrapper template ----

    template <typename T> class Wrap : public Napi::ObjectWrap<Wrap<T>> {
    public:
        T inner;

        Wrap(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Wrap<T>>(info) {}

        template <std::meta::info Fld>
        Napi::Value get_field(const Napi::CallbackInfo &info) {
            return to_napi(info.Env(), inner.[:Fld:]);
        }

        template <std::meta::info Fld>
        void set_field(const Napi::CallbackInfo &info, const Napi::Value &v) {
            using FieldT  = [:std::meta::type_of(Fld):];
            inner.[:Fld:] = from_napi<FieldT>(v);
        }

        template <std::meta::info Fld, rosetta::range R>
        void set_field_ranged(const Napi::CallbackInfo &info, const Napi::Value &v) {
            using FieldT        = [:std::meta::type_of(Fld):];
            constexpr auto name = std::define_static_string(std::meta::identifier_of(Fld));
            FieldT         val  = from_napi<FieldT>(v);
            double         d    = static_cast<double>(val);
            if (d < R.min || d > R.max) {
                throw Napi::RangeError::New(info.Env(), std::string(name) + " out of range");
            }
            inner.[:Fld:] = val;
        }

        template <std::meta::info Fld>
        void set_field_readonly(const Napi::CallbackInfo &info, const Napi::Value & /*v*/) {
            constexpr auto name = std::define_static_string(std::meta::identifier_of(Fld));
            throw Napi::TypeError::New(info.Env(), std::string(name) + " is read-only");
        }

        template <std::meta::info Fn>
        Napi::Value call_method(const Napi::CallbackInfo &info) {
            return call_method_impl<Fn>(
                info, std::make_index_sequence<
                          std::define_static_array(std::meta::parameters_of(Fn)).size()>{});
        }

        template <std::meta::info Fn>
        static Napi::Value call_static(const Napi::CallbackInfo &info) {
            return call_static_impl<Fn>(
                info, std::make_index_sequence<
                          std::define_static_array(std::meta::parameters_of(Fn)).size()>{});
        }

    private:
        template <std::meta::info Fn, std::size_t... Is>
        Napi::Value call_method_impl(const Napi::CallbackInfo &info,
                                     std::index_sequence<Is...>) {
            using R               = [:std::meta::return_type_of(Fn):];
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));
            if constexpr (std::is_void_v<R>) {
                (inner.[:Fn:])(
                    from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        info[Is])...);
                return info.Env().Undefined();
            } else {
                R r = (inner.[:Fn:])(
                    from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        info[Is])...);
                return to_napi(info.Env(), r);
            }
        }

        template <std::meta::info Fn, std::size_t... Is>
        static Napi::Value call_static_impl(const Napi::CallbackInfo &info,
                                            std::index_sequence<Is...>) {
            using R               = [:std::meta::return_type_of(Fn):];
            constexpr auto params = std::define_static_array(std::meta::parameters_of(Fn));
            if constexpr (std::is_void_v<R>) {
                ([:Fn:])(from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                    info[Is])...);
                return info.Env().Undefined();
            } else {
                R r = ([:Fn:])(
                    from_napi<std::remove_cvref_t<typename[:std::meta::type_of(params[Is]):]>>(
                        info[Is])...);
                return to_napi(info.Env(), r);
            }
        }
    };

    // ---- Visitor ----

    template <typename T> struct NapiVisitor {
        using This = Wrap<T>;
        std::vector<Napi::ClassPropertyDescriptor<This>> &props;

        template <std::meta::info Fld, auto... Anns> void field(const char *name) {
            using F = [:std::meta::type_of(Fld):];

            if constexpr (ann::has<readonly>(Anns...)) {
                props.push_back(
                    This::template InstanceAccessor<&This::template get_field<Fld>,
                                                    &This::template set_field_readonly<Fld>>(
                        name));
            } else if constexpr (ann::has<range>(Anns...) && std::is_arithmetic_v<F>) {
                constexpr auto r = ann::get_or<range>(range{0, 0}, Anns...);
                props.push_back(
                    This::template InstanceAccessor<&This::template get_field<Fld>,
                                                    &This::template set_field_ranged<Fld, r>>(
                        name));
            } else {
                props.push_back(
                    This::template InstanceAccessor<&This::template get_field<Fld>,
                                                    &This::template set_field<Fld>>(name));
            }
        }

        template <std::meta::info Fn, auto... /*Anns*/> void method_instance(const char *name) {
            props.push_back(This::template InstanceMethod<&This::template call_method<Fn>>(name));
        }

        template <std::meta::info Fn, auto... /*Anns*/> void method_static(const char *name) {
            props.push_back(This::template StaticMethod<&This::template call_static<Fn>>(name));
        }
    };

    template <typename T>
    Napi::Function bind_napi(Napi::Env env, const char *class_name) {
        using This = Wrap<T>;
        std::vector<Napi::ClassPropertyDescriptor<This>> props;
        NapiVisitor<T> v{props};
        walk<T>(v);
        return This::DefineClass(env, class_name, props);
    }

} // namespace rosetta
