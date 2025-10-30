// ============================================================================
// Header principal - Inclut toute la bibliothèque Rosetta
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

// Extensions (optionnelles)
#include "extensions/documentation/doc_generator.h"
#include "extensions/serialization/json_serializer.h"
#include "extensions/serialization/xml_serializer.h"
#include "extensions/validation/constraint_validator.h"

/**
 * @brief Namespace principal de Rosetta
 *
 * Rosetta est une bibliothèque d'introspection C++ qui permet :
 * - L'enregistrement non-intrusif de classes
 * - La génération automatique de bindings (Python, JavaScript, etc.)
 * - La sérialisation automatique (JSON, XML)
 * - La validation avec contraintes
 * - La génération de documentation
 */
namespace rosetta {

    // ============================================================================
    // Exports du namespace core
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
    // Exports du namespace extensions
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
    // Helpers pour simplifier l'utilisation
    // ============================================================================

    /**
     * @brief Helper pour créer une contrainte de range
     */
    template <typename T> auto make_range_constraint(T min, T max) {
        return extensions::make_range<T>(min, max);
    }

    /**
     * @brief Helper pour créer une contrainte not-null
     */
    template <typename T> auto make_not_null_constraint() {
        return extensions::make_not_null<T>();
    }

    /**
     * @brief Helper pour créer une contrainte de taille
     */
    template <typename Container> auto make_size_constraint(size_t min, size_t max = SIZE_MAX) {
        return extensions::make_size<Container>(min, max);
    }

    /**
     * @brief Helper pour créer une contrainte custom
     */
    template <typename T>
    auto make_custom_constraint(std::function<bool(const T &)> validator,
                                std::string                    error_message) {
        return extensions::make_custom<T>(std::move(validator), std::move(error_message));
    }

    // ============================================================================
    // Informations de version
    // ============================================================================

    /**
     * @brief Retourne la version de Rosetta
     * @return String au format "major.minor.patch"
     */
    inline std::string version() {
        return std::to_string(ROSETTA_VERSION_MAJOR) + "." + std::to_string(ROSETTA_VERSION_MINOR) +
               "." + std::to_string(ROSETTA_VERSION_PATCH);
    }

    /**
     * @brief Affiche les informations de la bibliothèque
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
// Macro helpers optionnels
// ============================================================================

/**
 * @brief Macro pour enregistrer rapidement une classe
 *
 * Usage:
 * ROSETTA_REGISTER_CLASS(MyClass)
 *     .field("x", &MyClass::x)
 *     .method("compute", &MyClass::compute);
 */
#define ROSETTA_REGISTER_CLASS(ClassName) \
    rosetta::Registry::instance().register_class<ClassName>(#ClassName)

/**
 * @brief Macro pour enregistrer une classe avec un nom custom
 */
#define ROSETTA_REGISTER_CLASS_AS(ClassName, Name) \
    rosetta::Registry::instance().register_class<ClassName>(Name)

/**
 * @brief Macro pour obtenir les métadonnées d'une classe
 */
#define ROSETTA_GET_META(ClassName) rosetta::Registry::instance().get<ClassName>()

/**
 * @brief Macro pour vérifier si une classe est enregistrée
 */
#define ROSETTA_HAS_CLASS(ClassName) rosetta::Registry::instance().has_class<ClassName>()

// ============================================================================
// Examples with comments
// ============================================================================

/*

// 1. Derfine your classes (no modification needed)
class Vector3D {
public:
    double x, y, z;

    Vector3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    double length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    void normalize() {
        double len = length();
        if (len > 0) {
            x /= len; y /= len; z /= len;
        }
    }
};

// 2. Register your classes
void setup_introspection() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);
}

// 3. Use introspection
void use_introspection() {
    Vector3D vec(3, 4, 0);

    // Dynamique access
    auto& meta = ROSETTA_GET_META(Vector3D);
    auto length = meta.invoke_method(vec, "length");
    std::cout << "Length: " << length.as<double>() << "\n";

    // Bindings generation
    rosetta::PythonGenerator py_gen;
    std::cout << py_gen.generate() << "\n";

    // Serialization
    std::string json = rosetta::JSONSerializer::serialize(vec);
    std::cout << "JSON: " << json << "\n";

    // Documentation
    rosetta::DocGenerator doc_gen;
    std::cout << doc_gen.generate() << "\n";
}

// 4. With inheritance
class Shape {
public:
    virtual double area() const = 0;
};

class Circle : public Shape {
public:
    double radius;

    double area() const override {
        return 3.14159 * radius * radius;
    }
};

void register_with_inheritance() {
    ROSETTA_REGISTER_CLASS(Shape)
        .pure_virtual_method<double>("area");

    ROSETTA_REGISTER_CLASS(Circle)
        .inherits_from<Shape>("Shape")
        .field("radius", &Circle::radius)
        .override_method("area", &Circle::area);
}

// 5. With validation
void setup_validation() {
    using namespace rosetta;

    ConstraintValidator::instance()
        .add_field_constraint<Circle, double>(
            "radius",
            make_range_constraint(0.0, 1000.0)
        );

    Circle c;
    c.radius = -5.0;

    std::vector<std::string> errors;
    if (!ConstraintValidator::instance().validate(c, errors)) {
        for (const auto& err : errors) {
            std::cerr << "Error: " << err << "\n";
        }
    }
}

*/