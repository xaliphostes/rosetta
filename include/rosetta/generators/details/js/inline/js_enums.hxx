/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#include "../js_common.h"
#include "../js_generator.h"

namespace rosetta {

    template <typename EnumType> inline void registerEnumType(JsGenerator &generator) {
        static_assert(std::is_enum_v<EnumType>, "Type must be an enum");

        const auto *enum_info = EnumRegistry::instance().getEnumInfo<EnumType>();
        if (!enum_info) {
            throw std::runtime_error("Enum not registered: " +
                                     std::string(typeid(EnumType).name()));
        }

        std::string type_name = enum_info->name;

        // Register bidirectional converters
        generator.register_type_converter(
            type_name,
            // C++ enum → JavaScript number
            [](Napi::Env env, const std::any &value) -> Napi::Value {
                try {
                    EnumType enum_val    = std::any_cast<EnumType>(value);
                    using UnderlyingType = std::underlying_type_t<EnumType>;
                    int64_t int_val = static_cast<int64_t>(static_cast<UnderlyingType>(enum_val));
                    return Napi::Number::New(env, static_cast<double>(int_val));
                } catch (const std::bad_any_cast &) {
                    return env.Undefined();
                }
            },
            // JavaScript value → C++ enum
            [enum_info](const Napi::Value &js_val) -> std::any {
                int64_t int_val;

                // Accept either number or string
                if (js_val.IsNumber()) {
                    int_val = static_cast<int64_t>(js_val.As<Napi::Number>().Int64Value());
                } else if (js_val.IsString()) {
                    std::string str_val = js_val.As<Napi::String>().Utf8Value();
                    int_val             = enum_info->getValue(str_val);
                } else {
                    throw std::runtime_error("Expected number or string for enum value");
                }

                // Validate that value exists
                if (!enum_info->hasValue(int_val)) {
                    throw std::runtime_error("Invalid enum value: " + std::to_string(int_val));
                }

                using UnderlyingType = std::underlying_type_t<EnumType>;
                return static_cast<EnumType>(static_cast<UnderlyingType>(int_val));
            });

        // Create JavaScript enum object
        auto enum_obj = Napi::Object::New(generator.env);

        // Add all enum values as properties
        for (const auto &value_info : enum_info->values) {
            enum_obj.Set(value_info.name,
                         Napi::Number::New(generator.env, static_cast<double>(value_info.value)));
        }

        // Make object immutable (freeze)
        auto global      = generator.env.Global();
        auto object_ctor = global.Get("Object").As<Napi::Object>();
        auto freeze_func = object_ctor.Get("freeze").As<Napi::Function>();
        freeze_func.Call({enum_obj});

        // Add to exports
        generator.exports.Set(type_name, enum_obj);
    }

    template <typename... EnumTypes> inline void registerEnumTypes(JsGenerator &generator) {
        (registerEnumType<EnumTypes>(generator), ...);
    }

    inline void bindAllEnums(JsGenerator &generator) {
        auto &registry = EnumRegistry::instance();

        for (const auto &enum_name : registry.getAllEnumNames()) {
            const auto *enum_info = registry.getEnumInfo(enum_name);
            if (!enum_info)
                continue;

            // Create JavaScript enum object
            auto enum_obj = Napi::Object::New(generator.env);

            // Add all enum values
            for (const auto &value_info : enum_info->values) {
                enum_obj.Set(
                    value_info.name,
                    Napi::Number::New(generator.env, static_cast<double>(value_info.value)));
            }

            // Freeze object
            auto global      = generator.env.Global();
            auto object_ctor = global.Get("Object").As<Napi::Object>();
            auto freeze_func = object_ctor.Get("freeze").As<Napi::Function>();
            freeze_func.Call({enum_obj});

            // Add to exports
            generator.exports.Set(enum_name, enum_obj);
        }
    }

} // namespace rosetta