/*
 * Copyright (c) 2025-now fmaerten@gmail.com
 * LGPL v3 license
 *
 * Overload helpers for registering overloaded methods in Rosetta.
 *
 * This header provides two approaches to simplify the verbose static_cast syntax
 * required when registering overloaded methods:
 *
 * 1. Template-based approach (recommended for type safety):
 *    .method("foo", rosetta::overload<void, MyClass, int>(&MyClass::foo))
 *
 * 2. Macro-based approach (shorter syntax):
 *    .method("foo", ROSETTA_OVERLOAD(MyClass, void, foo, int))
 */
#pragma once

namespace rosetta::core {

    // ============================================================================
    // TEMPLATE-BASED APPROACH (C++17)
    // ============================================================================

    /**
     * @brief Helper to select a specific overload of a non-const member function.
     *
     * Usage:
     *   .method("setBcType", rosetta::core::overload<void, BemSurface, int,
     * int>(&BemSurface::setBcType)) .method("setBcType", rosetta::core::overload<void, BemSurface,
     * const String&, const String&>(&BemSurface::setBcType))
     *
     * @tparam Ret    Return type of the method
     * @tparam Class  The class containing the method
     * @tparam Args   Parameter types of the method
     * @param ptr     Pointer to the member function
     * @return        The same pointer, with the correct type deduced
     */
    template <typename Ret, typename Class, typename... Args>
    constexpr auto overload(Ret (Class::*ptr)(Args...)) noexcept -> Ret (Class::*)(Args...) {
        return ptr;
    }

    /**
     * @brief Helper to select a specific overload of a const member function.
     *
     * Usage:
     *   .method("getValue", rosetta::core::overload_const<int, MyClass>(&MyClass::getValue))
     *   .method("getValue", rosetta::core::overload_const<double, MyClass>(&MyClass::getValue))
     *
     * @tparam Ret    Return type of the method
     * @tparam Class  The class containing the method
     * @tparam Args   Parameter types of the method
     * @param ptr     Pointer to the const member function
     * @return        The same pointer, with the correct type deduced
     */
    template <typename Ret, typename Class, typename... Args>
    constexpr auto overload_const(Ret (Class::*ptr)(Args...) const) noexcept
        -> Ret (Class::*)(Args...) const {
        return ptr;
    }

    /**
     * @brief Helper to select a specific overload of a static function.
     *
     * Usage:
     *   .static_method("create", rosetta::core::overload_static<MyClass*, int>(&MyClass::create))
     *   .static_method("create", rosetta::core::overload_static<MyClass*, const
     * String&>(&MyClass::create))
     *
     * @tparam Ret   Return type of the function
     * @tparam Args  Parameter types of the function
     * @param ptr    Pointer to the static function
     * @return       The same pointer, with the correct type deduced
     */
    template <typename Ret, typename... Args>
    constexpr auto overload_static(Ret (*ptr)(Args...)) noexcept -> Ret (*)(Args...) {
        return ptr;
    }

    // ============================================================================
    // ALTERNATIVE: Shorter aliases using 'sel' (select) naming
    // ============================================================================

    /**
     * @brief Short alias for overload (select member function)
     */
    template <typename Ret, typename Class, typename... Args>
    constexpr auto sel(Ret (Class::*ptr)(Args...)) noexcept -> Ret (Class::*)(Args...) {
        return ptr;
    }

    /**
     * @brief Short alias for overload_const (select const member function)
     */
    template <typename Ret, typename Class, typename... Args>
    constexpr auto sel_const(Ret (Class::*ptr)(Args...) const) noexcept
        -> Ret (Class::*)(Args...) const {
        return ptr;
    }

} // namespace rosetta::core

// ============================================================================
// MACRO-BASED APPROACH
// ============================================================================

/**
 * @brief Macro to simplify static_cast for non-const method overloads.
 *
 * Usage:
 *   .method("setBcType", ROSETTA_OVERLOAD(BemSurface, void, setBcType, int, int))
 *   .method("setBcType", ROSETTA_OVERLOAD(BemSurface, void, setBcType, const String&, const
 * String&))
 *
 * @param Class      The class containing the method
 * @param RetType    Return type of the method
 * @param MethodName Name of the method (without quotes)
 * @param ...        Parameter types (variadic)
 */
#define ROSETTA_OVERLOAD(Class, RetType, MethodName, ...) \
    static_cast<RetType (Class::*)(__VA_ARGS__)>(&Class::MethodName)

/**
 * @brief Macro to simplify static_cast for const method overloads.
 *
 * Usage:
 *   .method("getValue", ROSETTA_OVERLOAD_CONST(MyClass, int, getValue))
 *   .method("getValue", ROSETTA_OVERLOAD_CONST(MyClass, double, getValue, int))
 *
 * @param Class      The class containing the method
 * @param RetType    Return type of the method
 * @param MethodName Name of the method (without quotes)
 * @param ...        Parameter types (variadic, can be empty)
 */
#define ROSETTA_OVERLOAD_CONST(Class, RetType, MethodName, ...) \
    static_cast<RetType (Class::*)(__VA_ARGS__) const>(&Class::MethodName)

/**
 * @brief Macro to simplify static_cast for static method overloads.
 *
 * Usage:
 *   .static_method("create", ROSETTA_OVERLOAD_STATIC(MyClass, MyClass*, create, int))
 *   .static_method("create", ROSETTA_OVERLOAD_STATIC(MyClass, MyClass*, create, const String&))
 *
 * @param Class      The class containing the static method
 * @param RetType    Return type of the method
 * @param MethodName Name of the static method (without quotes)
 * @param ...        Parameter types (variadic)
 */
#define ROSETTA_OVERLOAD_STATIC(Class, RetType, MethodName, ...) \
    static_cast<RetType (*)(__VA_ARGS__)>(&Class::MethodName)

// ============================================================================
// SHORT MACRO ALIASES
// ============================================================================

/**
 * @brief Short alias for ROSETTA_OVERLOAD
 */
#define R_OVERLOAD(Class, RetType, MethodName, ...) \
    ROSETTA_OVERLOAD(Class, RetType, MethodName, ##__VA_ARGS__)

/**
 * @brief Short alias for ROSETTA_OVERLOAD_CONST
 */
#define R_OVERLOAD_CONST(Class, RetType, MethodName, ...) \
    ROSETTA_OVERLOAD_CONST(Class, RetType, MethodName, ##__VA_ARGS__)

/**
 * @brief Short alias for ROSETTA_OVERLOAD_STATIC
 */
#define R_OVERLOAD_STATIC(Class, RetType, MethodName, ...) \
    ROSETTA_OVERLOAD_STATIC(Class, RetType, MethodName, ##__VA_ARGS__)



// ============================================================================
// MACRO-BASED APPROACH - FREE FUNCTION OVERLOADS
// ============================================================================

/**
 * @brief Register an overloaded free function using the FULL function pointer type.
 *
 * Usage:
 *   ROSETTA_REGISTER_OVERLOADED_FUNCTION(triangulate, void(*)(SurfaceMesh&))
 *   ROSETTA_REGISTER_OVERLOADED_FUNCTION(read, void(*)(SurfaceMesh&, const std::filesystem::path&))
 *
 * @param FuncName   Name of the function (without quotes, will be stringified)
 * @param FuncPtrType Complete function pointer type, e.g., void(*)(int, double)
 */
// #define ROSETTA_REGISTER_OVERLOADED_FUNCTION(FuncName, FuncPtrType) \
//     rosetta::core::FunctionRegistry::instance().register_function(  \
//         #FuncName, static_cast<FuncPtrType>(&FuncName))

/**
 * @brief Register an overloaded free function with a custom name.
 *
 * Usage:
 *   ROSETTA_REGISTER_OVERLOADED_FUNCTION_AS(triangulate, "triangulate_mesh", void(*)(SurfaceMesh&))
 *   ROSETTA_REGISTER_OVERLOADED_FUNCTION_AS(triangulate, "triangulate_face", void(*)(SurfaceMesh&,
 * Face))
 *
 * @param FuncName    Name of the C++ function (without quotes)
 * @param AliasName   Name to register under (with quotes)
 * @param FuncPtrType Complete function pointer type
 */
// #define ROSETTA_REGISTER_OVERLOADED_FUNCTION_AS(FuncName, AliasName, FuncPtrType) \
//     rosetta::core::FunctionRegistry::instance().register_function(                \
//         AliasName, static_cast<FuncPtrType>(&FuncName))

// ============================================================================
// TEMPLATE-BASED REGISTRATION (Alternative - avoids macro issues entirely)
// ============================================================================

/**
 * @brief Register an overloaded free function using template syntax.
 *
 * Usage:
 *   rosetta_register_overloaded<void, SurfaceMesh&>("triangulate", &triangulate);
 *   rosetta_register_overloaded<void, SurfaceMesh&, Face>("triangulate_face", &triangulate);
 */
template <typename Ret, typename... Args>
inline void rosetta_register_overloaded(const char *name, Ret (*ptr)(Args...)) {
    rosetta::core::FunctionRegistry::instance().register_function(name, ptr);
}

// ============================================================================
// SHORT MACRO ALIASES
// ============================================================================

// #define R_OVERLOAD(Class, RetType, MethodName, ...) \
//     ROSETTA_OVERLOAD(Class, RetType, MethodName, ##__VA_ARGS__)

// #define R_OVERLOAD_CONST(Class, RetType, MethodName, ...) \
//     ROSETTA_OVERLOAD_CONST(Class, RetType, MethodName, ##__VA_ARGS__)

// #define R_OVERLOAD_STATIC(Class, RetType, MethodName, ...) \
//     ROSETTA_OVERLOAD_STATIC(Class, RetType, MethodName, ##__VA_ARGS__)

// #define R_REG_FUNC(FuncName, FuncPtrType) \
//     ROSETTA_REGISTER_OVERLOADED_FUNCTION(FuncName, FuncPtrType)

// #define R_REG_FUNC_AS(FuncName, AliasName, FuncPtrType) \
//     ROSETTA_REGISTER_OVERLOADED_FUNCTION_AS(FuncName, AliasName, FuncPtrType)
