/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include "js_generator.h"
#include <rosetta/type_registry.h>
#include <type_traits>
#include <typeinfo>

namespace rosetta {

    // ============================================================================
    // Type conversion helpers
    // ============================================================================

    template <typename T> inline Napi::Value toNapiValue(Napi::Env env, const T& value)
    {
        if constexpr (std::is_same_v<T, std::string>) {
            return Napi::String::New(env, value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return Napi::Boolean::New(env, value);
        } else if constexpr (std::is_integral_v<T>) {
            if constexpr (sizeof(T) > 4 || std::is_same_v<T, size_t>) {
                return Napi::Number::New(env, static_cast<double>(value));
            } else if constexpr (std::is_signed_v<T>) {
                return Napi::Number::New(env, static_cast<int32_t>(value));
            } else {
                return Napi::Number::New(env, static_cast<uint32_t>(value));
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            return Napi::Number::New(env, static_cast<double>(value));
        } else {
            return env.Undefined();
        }
    }

    template <typename T> inline T fromNapiValue(const Napi::Value& value)
    {
        if constexpr (std::is_same_v<T, std::string>) {
            if (!value.IsString()) {
                throw Napi::TypeError::New(value.Env(), "Expected string");
            }
            return value.As<Napi::String>().Utf8Value();
        } else if constexpr (std::is_same_v<T, bool>) {
            if (!value.IsBoolean()) {
                throw Napi::TypeError::New(value.Env(), "Expected boolean");
            }
            return value.As<Napi::Boolean>().Value();
        } else if constexpr (std::is_integral_v<T>) {
            if (!value.IsNumber()) {
                throw Napi::TypeError::New(value.Env(), "Expected number");
            }
            if constexpr (sizeof(T) > 4 || std::is_same_v<T, size_t>) {
                if constexpr (std::is_signed_v<T>) {
                    return static_cast<T>(value.As<Napi::Number>().Int64Value());
                } else {
                    return static_cast<T>(value.As<Napi::Number>().Int64Value());
                }
            } else if constexpr (std::is_signed_v<T>) {
                return static_cast<T>(value.As<Napi::Number>().Int32Value());
            } else {
                return static_cast<T>(value.As<Napi::Number>().Uint32Value());
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            if (!value.IsNumber()) {
                throw Napi::TypeError::New(value.Env(), "Expected number");
            }
            if constexpr (std::is_same_v<T, float>) {
                return static_cast<float>(value.As<Napi::Number>().FloatValue());
            } else {
                return value.As<Napi::Number>().DoubleValue();
            }
        } else {
            return T {};
        }
    }

} // namespace rosetta
