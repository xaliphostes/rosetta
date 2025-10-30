// ============================================================================
// type_info_example.cpp
//
// Example showing how to use the TypeInfo system for better type tracking
// ============================================================================

#include <iostream>
#include <rosetta/generators/js/js_generator.h>
#include <rosetta/generators/js/type_converters.h>
#include <rosetta/rosetta.h>

using namespace rosetta;
using namespace rosetta::generators::js;

// ============================================================================
// EXAMPLE CLASSES
// ============================================================================

class Vector3D {
public:
    double x, y, z;

    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}

    double length() const { return std::sqrt(x * x + y * y + z * z); }

    std::vector<double> to_array() const { return {x, y, z}; }
};

class DataContainer {
public:
    std::string           name;
    std::vector<int>      values;
    std::optional<double> threshold;

    DataContainer() : name(""), threshold(std::nullopt) {}
};

// ============================================================================
// TYPE INFO DEMONSTRATION
// ============================================================================

void demonstrate_type_info() {
    std::cout << "=== TypeInfo System Demonstration ===\n\n";

    // Create TypeInfo for various types
    TypeInfo int_info      = TypeInfo::create<int>();
    TypeInfo double_info   = TypeInfo::create<double>();
    TypeInfo string_info   = TypeInfo::create<std::string>();
    TypeInfo vector_info   = TypeInfo::create<std::vector<int>>();
    TypeInfo optional_info = TypeInfo::create<std::optional<double>>();
    TypeInfo class_info    = TypeInfo::create<Vector3D>();

    // Display information
    std::cout << "int type:\n";
    std::cout << "  Name: " << int_info.name << "\n";
    std::cout << "  Full name: " << int_info.full_name() << "\n";
    std::cout << "  Category: " << (int)int_info.category << "\n";
    std::cout << "  Size: " << int_info.size << " bytes\n";
    std::cout << "  Is numeric: " << int_info.is_numeric() << "\n";
    std::cout << "  Is integer: " << int_info.is_integer() << "\n\n";

    std::cout << "double type:\n";
    std::cout << "  Name: " << double_info.name << "\n";
    std::cout << "  Full name: " << double_info.full_name() << "\n";
    std::cout << "  Is numeric: " << double_info.is_numeric() << "\n";
    std::cout << "  Is floating: " << double_info.is_floating() << "\n\n";

    std::cout << "std::string type:\n";
    std::cout << "  Name: " << string_info.name << "\n";
    std::cout << "  Category: " << (int)string_info.category << "\n\n";

    std::cout << "std::vector<int> type:\n";
    std::cout << "  Name: " << vector_info.name << "\n";
    std::cout << "  Full name: " << vector_info.full_name() << "\n";
    std::cout << "  Is template: " << vector_info.is_template << "\n";
    std::cout << "  Template name: " << vector_info.template_name << "\n";
    std::cout << "  Template args count: " << vector_info.template_args.size() << "\n";
    if (!vector_info.template_args.empty()) {
        std::cout << "  Element type: " << vector_info.template_args[0].name << "\n";
    }
    std::cout << "\n";

    std::cout << "std::optional<double> type:\n";
    std::cout << "  Name: " << optional_info.name << "\n";
    std::cout << "  Full name: " << optional_info.full_name() << "\n";
    std::cout << "  Is template: " << optional_info.is_template << "\n";
    std::cout << "  Template name: " << optional_info.template_name << "\n";
    if (!optional_info.template_args.empty()) {
        std::cout << "  Value type: " << optional_info.template_args[0].name << "\n";
    }
    std::cout << "\n";

    std::cout << "Vector3D type:\n";
    std::cout << "  Name: " << class_info.name << "\n";
    std::cout << "  Category: " << (int)class_info.category << "\n";
    std::cout << "  Size: " << class_info.size << " bytes\n";
    std::cout << "  Alignment: " << class_info.alignment << " bytes\n\n";
}

// ============================================================================
// TYPE REGISTRY DEMONSTRATION
// ============================================================================

void demonstrate_type_registry() {
    std::cout << "=== TypeRegistry Demonstration ===\n\n";

    auto &registry = TypeRegistry::instance();

    // Register types with custom names
    registry.register_type<int>("integer");
    registry.register_type<double>("number");
    registry.register_type<std::string>("text");
    registry.register_type<Vector3D>("Vector3D");
    registry.register_type<std::vector<double>>("double_array");

    // List all registered types
    std::cout << "Registered types:\n";
    for (const auto &name : registry.list_types()) {
        std::cout << "  - " << name << "\n";
    }
    std::cout << "\n";

    // Get TypeInfo by type
    const TypeInfo &int_info = registry.get<int>();
    std::cout << "TypeInfo for int:\n";
    std::cout << "  Name: " << int_info.name << "\n";
    std::cout << "  Category: " << (int)int_info.category << "\n\n";

    // Get TypeInfo by name
    const TypeInfo *vec_info = registry.get_by_name("Vector3D");
    if (vec_info) {
        std::cout << "TypeInfo for 'Vector3D':\n";
        std::cout << "  Full name: " << vec_info->full_name() << "\n";
        std::cout << "  Size: " << vec_info->size << " bytes\n\n";
    }

    // Check if type is registered
    std::cout << "Is Vector3D registered? " << registry.has_type<Vector3D>() << "\n";
    std::cout << "Is int registered? " << registry.has_type<int>() << "\n\n";
}

// ============================================================================
// ROSETTA REGISTRATION
// ============================================================================

void register_classes() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("to_array", &Vector3D::to_array);

    ROSETTA_REGISTER_CLASS(DataContainer)
        .field("name", &DataContainer::name)
        .field("values", &DataContainer::values)
        .field("threshold", &DataContainer::threshold);
}

// ============================================================================
// N-API BINDING WITH TYPE INFO
// ============================================================================

BEGIN_JS_MODULE(gen) {
    // Run demonstrations (in real code, remove these)
    // demonstrate_type_info();
    // demonstrate_type_registry();

    // Register classes
    register_classes();

    // Register type converters (these now use TypeInfo internally)
    register_vector_converter<int>(gen);
    register_vector_converter<double>(gen);
    register_optional_converter<double>(gen);

    // Bind classes
    gen.bind_classes<Vector3D, DataContainer>();

    // Add custom converter with TypeInfo
    gen.register_converter<Vector3D>(
        // C++ to JS
        [](Napi::Env env, const core::Any &val) -> Napi::Value {
            const auto  &vec = val.as<Vector3D>();
            Napi::Object obj = Napi::Object::New(env);
            obj.Set("x", Napi::Number::New(env, vec.x));
            obj.Set("y", Napi::Number::New(env, vec.y));
            obj.Set("z", Napi::Number::New(env, vec.z));
            return obj;
        },
        // JS to C++
        [](const Napi::Value &val) -> core::Any {
            if (!val.IsObject())
                return core::Any();
            Napi::Object obj = val.As<Napi::Object>();
            Vector3D     vec;
            vec.x = obj.Get("x").As<Napi::Number>().DoubleValue();
            vec.y = obj.Get("y").As<Napi::Number>().DoubleValue();
            vec.z = obj.Get("z").As<Napi::Number>().DoubleValue();
            return core::Any(vec);
        });

    // Add utility to inspect types
    auto inspect_type = [](const Napi::CallbackInfo &info) -> Napi::Value {
        Napi::Env env = info.Env();

        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(env, "Expected type name as string").ThrowAsJavaScriptException();
            return env.Undefined();
        }

        std::string     type_name = info[0].As<Napi::String>().Utf8Value();
        const TypeInfo *type_info = TypeRegistry::instance().get_by_name(type_name);

        if (!type_info) {
            return env.Null();
        }

        Napi::Object result = Napi::Object::New(env);
        result.Set("name", Napi::String::New(env, type_info->name));
        result.Set("fullName", Napi::String::New(env, type_info->full_name()));
        result.Set("size", Napi::Number::New(env, type_info->size));
        result.Set("alignment", Napi::Number::New(env, type_info->alignment));
        result.Set("isTemplate", Napi::Boolean::New(env, type_info->is_template));
        result.Set("isConst", Napi::Boolean::New(env, type_info->is_const));
        result.Set("isPointer", Napi::Boolean::New(env, type_info->is_pointer));
        result.Set("isNumeric", Napi::Boolean::New(env, type_info->is_numeric()));

        return result;
    };

    gen.exports.Set("inspectType", Napi::Function::New(gen.env, inspect_type, "inspectType"));
    gen.add_utilities();
}
END_JS_MODULE();