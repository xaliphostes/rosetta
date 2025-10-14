// ============================================================================
// rosetta/generators/napi_binding_generator.hpp
// 
// Générateur de bindings JavaScript avec Node-API (NAPI)
// ============================================================================
#pragma once
#include "../core/registry.h"
#include <node_api.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <stdexcept>
#include <cmath>

namespace rosetta::generators {

/**
 * @brief Générateur de bindings JavaScript utilisant Node-API
 * 
 * Usage similaire à PythonBindingGenerator mais pour Node.js
 */
class NapiBindingGenerator {
    napi_env env_;
    napi_value exports_;
    
    // Cache des constructeurs de classes
    std::unordered_map<std::string, napi_ref> class_constructors_;
    
public:
    /**
     * @brief Constructeur
     * @param env Environnement Node-API
     * @param exports Objet exports du module
     */
    NapiBindingGenerator(napi_env env, napi_value exports)
        : env_(env), exports_(exports) {}
    
    /**
     * @brief Destructeur - libère les références
     */
    ~NapiBindingGenerator() {
        for (auto& [name, ref] : class_constructors_) {
            napi_delete_reference(env_, ref);
        }
    }
    
    // ========================================================================
    // BIND CLASS
    // ========================================================================
    
    /**
     * @brief Lie une classe introspectée automatiquement
     * @tparam Class Type de la classe
     * @param custom_name Nom custom (optionnel)
     */
    template<typename Class>
    NapiBindingGenerator& bind_class(const std::string& custom_name = "") {
        auto& registry = core::Registry::instance();
        
        if (!registry.has_class<Class>()) {
            throw std::runtime_error("Class not registered in Rosetta: " + 
                                    std::string(typeid(Class).name()));
        }
        
        const auto& meta = registry.get<Class>();
        const auto& inheritance = meta.inheritance();
        
        std::string class_name = custom_name.empty() ? meta.name() : custom_name;
        
        // Créer la classe JavaScript
        create_class<Class>(class_name, meta, inheritance);
        
        return *this;
    }
    
    // ========================================================================
    // BIND MULTIPLE CLASSES
    // ========================================================================
    
    /**
     * @brief Lie plusieurs classes en une seule ligne
     */
    template<typename... Classes>
    NapiBindingGenerator& bind_classes() {
        (bind_class<Classes>(), ...);
        return *this;
    }
    
    // ========================================================================
    // BIND FUNCTION
    // ========================================================================
    
    /**
     * @brief Lie une fonction libre
     * @tparam Ret Type de retour
     * @tparam Args Types des arguments
     * @param func Pointeur vers la fonction
     * @param name Nom en JavaScript
     */
    template<typename Ret, typename... Args>
    NapiBindingGenerator& bind_function(Ret (*func)(Args...), 
                                       const std::string& name) {
        // Wrapper NAPI pour la fonction
        auto callback = [](napi_env env, napi_callback_info info) -> napi_value {
            // Récupérer le pointeur de fonction depuis data
            void* data;
            napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
            
            auto* func_ptr = static_cast<Ret(*)(Args...)>(data);
            
            // TODO: Extraire les arguments, appeler la fonction, retourner le résultat
            
            napi_value result;
            napi_get_undefined(env, &result);
            return result;
        };
        
        // Créer la fonction NAPI
        napi_value js_function;
        napi_create_function(env_, name.c_str(), NAPI_AUTO_LENGTH,
                           callback, reinterpret_cast<void*>(func), &js_function);
        
        // Ajouter aux exports
        napi_set_named_property(env_, exports_, name.c_str(), js_function);
        
        return *this;
    }
    
    // ========================================================================
    // BIND ENUM
    // ========================================================================
    
    /**
     * @brief Lie une énumération
     * @tparam Enum Type de l'énumération
     * @param name Nom en JavaScript
     * @param values Paires {nom, valeur}
     */
    template<typename Enum>
    NapiBindingGenerator& bind_enum(const std::string& name,
                                    std::initializer_list<std::pair<const char*, Enum>> values) {
        static_assert(std::is_enum_v<Enum>, "Type must be an enum");
        
        // Créer un objet JavaScript pour l'enum
        napi_value enum_obj;
        napi_create_object(env_, &enum_obj);
        
        for (const auto& [value_name, value] : values) {
            napi_value js_value;
            napi_create_int32(env_, static_cast<int32_t>(value), &js_value);
            napi_set_named_property(env_, enum_obj, value_name, js_value);
        }
        
        // Ajouter aux exports
        napi_set_named_property(env_, exports_, name.c_str(), enum_obj);
        
        return *this;
    }

private:
    // ========================================================================
    // HELPERS INTERNES
    // ========================================================================
    
    template<typename Class>
    void create_class(const std::string& name,
                     const core::ClassMetadata<Class>& meta,
                     const core::InheritanceInfo& inheritance) {
        
        std::vector<napi_property_descriptor> properties;
        
        // Ajouter les champs comme propriétés
        for (const auto& field_name : meta.fields()) {
            properties.push_back(create_field_property<Class>(field_name, meta));
        }
        
        // Ajouter les méthodes
        for (const auto& method_name : meta.methods()) {
            properties.push_back(create_method_property<Class>(method_name, meta));
        }
        
        // Créer le constructeur
        napi_value constructor;
        napi_define_class(env_, name.c_str(), NAPI_AUTO_LENGTH,
                         constructor_callback<Class>, nullptr,
                         properties.size(), properties.data(),
                         &constructor);
        
        // Stocker une référence au constructeur
        napi_ref constructor_ref;
        napi_create_reference(env_, constructor, 1, &constructor_ref);
        class_constructors_[name] = constructor_ref;
        
        // Ajouter aux exports
        napi_set_named_property(env_, exports_, name.c_str(), constructor);
    }
    
    // Callback pour le constructeur
    template<typename Class>
    static napi_value constructor_callback(napi_env env, napi_callback_info info) {
        napi_value js_this;
        napi_get_cb_info(env, info, nullptr, nullptr, &js_this, nullptr);
        
        // Créer une instance C++
        Class* instance = new Class();
        
        // Wrapper l'instance dans l'objet JavaScript
        napi_wrap(env, js_this, instance,
                 [](napi_env env, void* data, void* hint) {
                     delete static_cast<Class*>(data);
                 },
                 nullptr, nullptr);
        
        return js_this;
    }
    
    // Créer une propriété pour un champ
    template<typename Class>
    napi_property_descriptor create_field_property(
        const std::string& field_name,
        const core::ClassMetadata<Class>& meta) {
        
        // Getter
        auto getter = [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value js_this;
            void* data;
            napi_get_cb_info(env, info, nullptr, nullptr, &js_this, &data);
            
            // Récupérer l'instance C++
            Class* instance;
            napi_unwrap(env, js_this, reinterpret_cast<void**>(&instance));
            
            // Récupérer le nom du champ depuis data
            auto* field_info = static_cast<FieldInfo<Class>*>(data);
            auto value = field_info->meta->get_field(*instance, field_info->name);
            
            // Convertir en JavaScript
            return any_to_napi(env, value);
        };
        
        // Setter
        auto setter = [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value js_this, js_value;
            size_t argc = 1;
            void* data;
            napi_get_cb_info(env, info, &argc, &js_value, &js_this, &data);
            
            // Récupérer l'instance C++
            Class* instance;
            napi_unwrap(env, js_this, reinterpret_cast<void**>(&instance));
            
            // Récupérer le nom du champ depuis data
            auto* field_info = static_cast<FieldInfo<Class>*>(data);
            auto cpp_value = napi_to_any(env, js_value);
            
            field_info->meta->set_field(*instance, field_info->name, cpp_value);
            
            napi_value undefined;
            napi_get_undefined(env, &undefined);
            return undefined;
        };
        
        // Stocker les infos du champ
        auto* field_info = new FieldInfo<Class>{&meta, field_name};
        
        return napi_property_descriptor{
            field_name.c_str(),
            nullptr,
            nullptr,
            getter,
            setter,
            nullptr,
            napi_default,
            field_info
        };
    }
    
    // Créer une propriété pour une méthode
    template<typename Class>
    napi_property_descriptor create_method_property(
        const std::string& method_name,
        const core::ClassMetadata<Class>& meta) {
        
        auto method = [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value js_this;
            size_t argc = 10; // Max arguments
            napi_value args[10];
            void* data;
            napi_get_cb_info(env, info, &argc, args, &js_this, &data);
            
            // Récupérer l'instance C++
            Class* instance;
            napi_unwrap(env, js_this, reinterpret_cast<void**>(&instance));
            
            // Récupérer le nom de la méthode depuis data
            auto* method_info = static_cast<MethodInfo<Class>*>(data);
            
            // Convertir les arguments
            std::vector<core::Any> cpp_args;
            for (size_t i = 0; i < argc; ++i) {
                cpp_args.push_back(napi_to_any(env, args[i]));
            }
            
            // Invoquer la méthode
            auto result = method_info->meta->invoke_method(*instance, 
                                                          method_info->name, 
                                                          cpp_args);
            
            // Convertir le résultat
            return any_to_napi(env, result);
        };
        
        // Stocker les infos de la méthode
        auto* method_info = new MethodInfo<Class>{&meta, method_name};
        
        return napi_property_descriptor{
            method_name.c_str(),
            nullptr,
            method,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            method_info
        };
    }
    
    // Structures pour stocker les infos
    template<typename Class>
    struct FieldInfo {
        const core::ClassMetadata<Class>* meta;
        std::string name;
    };
    
    template<typename Class>
    struct MethodInfo {
        const core::ClassMetadata<Class>* meta;
        std::string name;
    };
    
    // Conversion Any → NAPI
    static napi_value any_to_napi(napi_env env, const core::Any& value) {
        // Simplification - nécessite inspection du type réel
        if (!value.has_value()) {
            napi_value result;
            napi_get_undefined(env, &result);
            return result;
        }
        
        // Essayer différents types
        // Note: Ceci est une simplification - une vraie implémentation
        // nécessiterait un système de type registry
        
        napi_value result;
        
        try {
            // Essayer double
            double d = const_cast<core::Any&>(value).as<double>();
            napi_create_double(env, d, &result);
            return result;
        } catch (...) {}
        
        try {
            // Essayer int
            int i = const_cast<core::Any&>(value).as<int>();
            napi_create_int32(env, i, &result);
            return result;
        } catch (...) {}
        
        try {
            // Essayer bool
            bool b = const_cast<core::Any&>(value).as<bool>();
            napi_get_boolean(env, b, &result);
            return result;
        } catch (...) {}
        
        try {
            // Essayer string
            std::string s = const_cast<core::Any&>(value).as<std::string>();
            napi_create_string_utf8(env, s.c_str(), s.length(), &result);
            return result;
        } catch (...) {}
        
        // Par défaut, retourner undefined
        napi_get_undefined(env, &result);
        return result;
    }
    
    // Conversion NAPI → Any
    static core::Any napi_to_any(napi_env env, napi_value value) {
        napi_valuetype type;
        napi_typeof(env, value, &type);
        
        switch (type) {
            case napi_number: {
                double num;
                napi_get_value_double(env, value, &num);
                return core::Any(num);
            }
            case napi_string: {
                size_t length;
                napi_get_value_string_utf8(env, value, nullptr, 0, &length);
                std::string str(length, '\0');
                napi_get_value_string_utf8(env, value, &str[0], length + 1, nullptr);
                return core::Any(str);
            }
            case napi_boolean: {
                bool b;
                napi_get_value_bool(env, value, &b);
                return core::Any(b);
            }
            default:
                return core::Any(0);
        }
    }
};

} // namespace rosetta::generators

// ============================================================================
// MACROS HELPER
// ============================================================================

/**
 * @brief Macro pour initialiser un module Node.js
 */
#define BEGIN_MODULE(module_name) \
    napi_value Init_##module_name(napi_env env, napi_value exports)

/**
 * @brief Macro pour enregistrer le module
 */
#define END_MODULE(module_name) \
    NAPI_MODULE(NODE_GYP_MODULE_NAME, Init_##module_name)

/**
 * @brief Macro pour lier automatiquement une classe
 */
#define NAPI_AUTO_BIND_CLASS(env, exports, Class) \
    rosetta::generators::NapiBindingGenerator(env, exports).bind_class<Class>()

/**
 * @brief Macro pour lier plusieurs classes
 */
#define NAPI_AUTO_BIND_CLASSES(env, exports, ...) \
    rosetta::generators::NapiBindingGenerator(env, exports).bind_classes<__VA_ARGS__>()

// ============================================================================
// EXEMPLE D'UTILISATION COMPLET
// ============================================================================

/*

#include <node_api.h>
#include "rosetta/rosetta.hpp"
#include "rosetta/generators/napi_binding_generator.hpp"

// 1. Vos classes (aucune modification)
class Vector3D {
public:
    double x, y, z;
    
    Vector3D(double x = 0, double y = 0, double z = 0) 
        : x(x), y(y), z(z) {}
    
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

class Point2D {
public:
    double x, y;
    
    Point2D(double x = 0, double y = 0) : x(x), y(y) {}
    
    double distance() const {
        return std::sqrt(x*x + y*y);
    }
};

enum class Status {
    Active,
    Inactive,
    Pending
};

// 2. Enregistrement Rosetta
void register_types() {
    ROSETTA_REGISTER_CLASS(Vector3D)
        .field("x", &Vector3D::x)
        .field("y", &Vector3D::y)
        .field("z", &Vector3D::z)
        .method("length", &Vector3D::length)
        .method("normalize", &Vector3D::normalize);
    
    ROSETTA_REGISTER_CLASS(Point2D)
        .field("x", &Point2D::x)
        .field("y", &Point2D::y)
        .method("distance", &Point2D::distance);
}

// 3. Module Node.js avec binding automatique
NAPI_MODULE_INIT(geometry) {
    // Enregistrer les types d'abord
    register_types();
    
    // Créer le générateur
    rosetta::generators::NapiBindingGenerator generator(env, exports);
    
    // Lier les classes automatiquement
    generator.bind_class<Vector3D>();
    generator.bind_class<Point2D>("Point");  // Nom custom
    
    // Ou lier plusieurs à la fois
    // generator.bind_classes<Vector3D, Point2D>();
    
    // Lier une enum
    generator.bind_enum<Status>("Status", {
        {"Active", Status::Active},
        {"Inactive", Status::Inactive},
        {"Pending", Status::Pending}
    });
    
    // Ou utiliser la macro
    // NAPI_AUTO_BIND_CLASS(env, exports, Vector3D);
    
    return exports;
}

NAPI_MODULE_REGISTER(geometry)

// 4. Fichier binding.gyp pour compilation
{
  "targets": [
    {
      "target_name": "geometry",
      "sources": [ "bindings.cpp" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "path/to/rosetta"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "cflags_cc": [ "-std=c++17" ]
    }
  ]
}

// 5. Compilation
// $ npm install
// $ node-gyp configure
// $ node-gyp build

// 6. Utilisation en Node.js
const geometry = require('./build/Release/geometry');

const v = new geometry.Vector3D();
v.x = 3.0;
v.y = 4.0;
v.z = 0.0;

console.log(v.length());  // 5.0
v.normalize();
console.log(v.x, v.y, v.z);  // 0.6, 0.8, 0.0

const p = new geometry.Point();
p.x = 3;
p.y = 4;
console.log(p.distance());  // 5.0

console.log(geometry.Status.Active);  // 0

*/