/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 */
#pragma once
#include <functional>
#include <rosetta/info.h>
#include <rosetta/types.h>
#include <string>
#include <unordered_map>

namespace rosetta {

    /**
     * @brief Information about a standalone function
     */
    class FunctionInfo {
    public:
        std::string                           name;
        std::string                           return_type;
        std::vector<std::string>              parameter_types;
        std::function<std::any(const Args &)> invoker;

        FunctionInfo(const std::string &n, const std::string &ret_type,
                     const std::vector<std::string>       &param_types,
                     std::function<std::any(const Args &)> inv)
            : name(n), return_type(ret_type), parameter_types(param_types), invoker(inv) {}
    };

    /**
     * @brief Registry for standalone functions
     */
    class FunctionRegistry {
    public:
        static FunctionRegistry &instance();

        void                     registerFunction(std::unique_ptr<FunctionInfo> func);
        const FunctionInfo      *getFunction(const std::string &name) const;
        std::vector<std::string> getFunctionNames() const;

    private:
        std::unordered_map<std::string, std::unique_ptr<FunctionInfo>> functions;
    };

    inline FunctionRegistry &FunctionRegistry::instance() {
        static FunctionRegistry registry;
        return registry;
    }

    inline void FunctionRegistry::registerFunction(std::unique_ptr<FunctionInfo> func) {
        functions[func->name] = std::move(func);
    }

    inline const FunctionInfo *FunctionRegistry::getFunction(const std::string &name) const {
        auto it = functions.find(name);
        return (it != functions.end()) ? it->second.get() : nullptr;
    }

    inline std::vector<std::string> FunctionRegistry::getFunctionNames() const {
        std::vector<std::string> names;
        for (const auto &[name, _] : functions) {
            names.push_back(name);
        }
        return names;
    }

    /**
     * @brief Helper to register functions with type deduction
     */
    template <typename ReturnType, typename... Args>
    inline FunctionRegistrar<ReturnType, Args...>::FunctionRegistrar(
        const std::string &name, ReturnType (*func_ptr)(Args...)) {
        auto info = std::make_unique<FunctionInfo>(
            name, getTypeName<ReturnType>(), std::vector<std::string>{getTypeName<Args>()...},
            [func_ptr](const std::vector<std::any> &args) -> std::any {
                if (args.size() != sizeof...(Args)) {
                    throw std::runtime_error("Wrong number of arguments");
                }
                return callFunctionImpl(func_ptr, args, std::index_sequence_for<Args...>{});
            });
        FunctionRegistry::instance().registerFunction(std::move(info));
    }

    template <typename ReturnType, typename... Args>
    template <std::size_t... I>
    inline std::any FunctionRegistrar<ReturnType, Args...>::callFunctionImpl(
        ReturnType (*func_ptr)(Args...), const std::vector<std::any> &args,
        std::index_sequence<I...>) {
        if constexpr (std::is_void_v<ReturnType>) {
            func_ptr(std::any_cast<Args>(args[I])...);
            return std::any{};
        } else {
            return std::any{func_ptr(std::any_cast<Args>(args[I])...)};
        }
    }

} // namespace rosetta
