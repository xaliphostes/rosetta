// ============================================================================
// Main Header - Includes all necessary Rosetta headers
// ============================================================================
#pragma once

// Version
#define ROSETTA_VERSION_MAJOR 1
#define ROSETTA_VERSION_MINOR 0
#define ROSETTA_VERSION_PATCH 0

#include <iostream>

// Core
#include "core/any.h"
#include "core/class_metadata.h"
#include "core/inheritance_info.h"
#include "core/registry.h"
#include "core/type_kind.h"
#include "core/virtual_method_info.h"
#include "core/virtual_method_registry.h"

// Traits
#include "traits/container_traits.h"
#include "traits/inheritance_traits.h"
#include "traits/pointer_traits.h"

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
    using core::InheritanceInfo;
    using core::InheritanceType;
    using core::Registry;
    using core::TypeKind;
    using core::VirtualMethodInfo;
    using core::VirtualMethodRegistry;
    using core::VirtualTableInfo;

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

    // ============================================================================
    // Version and Info
    // ============================================================================

    /**
     * @brief Get the Rosetta version as a string
     * @return String with format "major.minor.patch"
     */
    inline std::string version() {
        return std::to_string(ROSETTA_VERSION_MAJOR) + "." + std::to_string(ROSETTA_VERSION_MINOR) +
               "." + std::to_string(ROSETTA_VERSION_PATCH);
    }

    /**
     * @brief Display Rosetta information to stdout
     */
    inline void print_info() {
        std::cout << "Rosetta C++ Introspection Library\n";
        std::cout << "Version: " << version() << "\n";
        std::cout << "Features:\n";
        std::cout << "  - Non-intrusive class registration\n";
        std::cout << "  - Automatic binding generation (Python, JS, TS)\n";
        std::cout << "  - Serialization (JSON, XML)\n";
        std::cout << "  - Validation with constraints\n";
        std::cout << "  - Documentation generation\n";
    }

} // namespace rosetta

// ============================================================================
// Optional Macros for Simplified Usage
// ============================================================================

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
