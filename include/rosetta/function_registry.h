/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <any>
#include <string>
#include <unordered_map>

namespace rosetta {

    /**
     * @brief Helper to register functions with type deduction
     */
    template <typename ReturnType, typename... Args> class FunctionRegistrar {
    public:
        FunctionRegistrar(const std::string &name, ReturnType (*func_ptr)(Args...));

    private:
        template <std::size_t... I>
        static std::any callFunctionImpl(ReturnType (*func_ptr)(Args...),
                                         const std::vector<std::any> &args,
                                         std::index_sequence<I...>);
    };

} // namespace rosetta

/**
 * @brief Macro to register a function
 * @example
 * int add(int a, int b) { return a + b; }
 * REGISTER_FUNCTION(add);
 */
#define REGISTER_FUNCTION(func)                                           \
    namespace {                                                           \
        static rosetta::FunctionRegistrar func##_registrar(#func, &func); \
    }

#include "inline/function_registry.hxx"