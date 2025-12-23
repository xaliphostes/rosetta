// ============================================================================
// Main Header - Includes all necessary Rosetta headers
// ============================================================================
#pragma once

// Version
#define ROSETTA_VERSION_MAJOR 1
#define ROSETTA_VERSION_MINOR 0
#define ROSETTA_VERSION_PATCH 0

// Core
#include "core/version.h"
#include "core/any.h"
#include "core/overload.h"
#include "core/class_metadata.h"
#include "core/demangler.h"
#include "core/function_metadata.h"
#include "core/function_registry.h"
#include "core/inheritance_info.h"
#include "core/registry.h"
#include "core/type_kind.h"
#include "core/virtual_method_info.h"
#include "core/virtual_method_registry.h"

// Extensions
#include "extensions/documentation/doc_generator.h"
#include "extensions/serialization/json_serializer.h"
#include "extensions/serialization/xml_serializer.h"
#include "extensions/validation/constraint_validator.h"
// DO NOT INCLUDE "generators"!!!
// (as we will have to include the tird party libs for the generators)

namespace rosetta {

    // ============================================================================
    // Exports of core namespace
    // ============================================================================

    using core::AccessSpecifier;
    using core::Any;
    using core::BaseClassInfo;
    using core::ClassMetadata;
    using core::demangle;
    using core::FunctionMetadata;
    using core::FunctionRegistry;
    using core::get_readable_type_name;
    using core::InheritanceInfo;
    using core::InheritanceType;
    using core::Registry;
    using core::TypeKind;
    using core::TypeNameRegistry;
    using core::VirtualMethodInfo;
    using core::VirtualMethodRegistry;
    using core::VirtualTableInfo;
    using core::print_info;
    using core::version;
    using core::overload;
    using core::overload_const;
    using core::overload_static;
    using core::sel;
    using core::sel_const;

    // ============================================================================
    // Exports of extensions namespace
    // ============================================================================

    using extensions::Constraint;
    using extensions::ConstraintValidator;
    using extensions::CustomConstraint;
    using extensions::DocFormat;
    using extensions::DocGenerator;
    using extensions::JSONSerializer;
    using extensions::NotNullConstraint;
    using extensions::RangeConstraint;
    using extensions::SizeConstraint;
    using extensions::XMLSerializer;

    // ============================================================================
    // Helpers to simplify constraint creation
    // ============================================================================

    /**
     * @brief Helper to create a range constraint
     */
    template <typename T> auto make_range_constraint(T min, T max) {
        return extensions::make_range<T>(min, max);
    }

    /**
     * @brief Helper to create a not-null constraint
     */
    template <typename T> auto make_not_null_constraint() {
        return extensions::make_not_null<T>();
    }

    /**
     * @brief Helper to create a size constraint
     */
    template <typename Container> auto make_size_constraint(size_t min, size_t max = SIZE_MAX) {
        return extensions::make_size<Container>(min, max);
    }

    /**
     * @brief Helper to create a custom constraint
     */
    template <typename T>
    auto make_custom_constraint(std::function<bool(const T &)> validator,
                                std::string                    error_message) {
        return extensions::make_custom<T>(std::move(validator), std::move(error_message));
    }

} // namespace rosetta

// ===============================================================
//                        C L A S S E S
// ===============================================================

/**
 * @brief Register a class with Rosetta using its type name
 *
 * Usage:
 * ROSETTA_REGISTER_CLASS(MyClass)
 *     .field("x", &MyClass::x)
 *     .method("compute", &MyClass::compute);
 */
#define ROSETTA_REGISTER_CLASS(ClassName) \
    rosetta::Registry::instance().register_class<ClassName>(#ClassName)

/**
 * @brief Register a class with a custom name
 */
#define ROSETTA_REGISTER_CLASS_AS(ClassName, Name) \
    rosetta::Registry::instance().register_class<ClassName>(Name)

/**
 * @brief Get metadata for a registered class
 */
#define ROSETTA_GET_META(ClassName) rosetta::Registry::instance().get<ClassName>()

/**
 * @brief Check if a class is registered
 */
#define ROSETTA_HAS_CLASS(ClassName) rosetta::Registry::instance().has_class<ClassName>()

// ===============================================================
//                        F U N C T I O N S
// ===============================================================

/**
 * @brief Register a free function with Rosetta
 *
 * Usage:
 * ROSETTA_REGISTER_FUNCTION(my_function);
 */
#define ROSETTA_REGISTER_FUNCTION(FuncName) \
    rosetta::core::FunctionRegistry::instance().register_function(#FuncName, &FuncName)

/**
 * @brief Register a free function with a custom name
 */
#define ROSETTA_REGISTER_FUNCTION_AS(FuncName, Name) \
    rosetta::core::FunctionRegistry::instance().register_function(Name, &FuncName)

/**
 * @brief Get metadata for a registered function
 */
#define ROSETTA_GET_FUNCTION(FuncName) rosetta::core::FunctionRegistry::instance().get(#FuncName)

/**
 * @brief Check if a function is registered
 */
#define ROSETTA_HAS_FUNCTION(FuncName) \
    rosetta::core::FunctionRegistry::instance().has_function(#FuncName)
