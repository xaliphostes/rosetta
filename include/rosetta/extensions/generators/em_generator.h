// ============================================================================
// Emscripten binding generator using Rosetta introspection
// Auto-bind inherited methods (no JS enhancement needed)
// ============================================================================
#pragma once

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <functional>
#include <map>
#include <rosetta/rosetta.h>
#include <set>
#include <sstream>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// namespace em = emscripten;

namespace rosetta::em {

    class EmGenerator {
    public:
        EmGenerator() = default;

        template <typename T, typename... CtorSignatures>
        EmGenerator &bind_class(const std::string &js_name = "");

        template <typename T, typename FactoryFunc>
        EmGenerator &bind_class_with_factory(FactoryFunc factory, const std::string &js_name = "");

        template <typename Derived, typename Base, typename... CtorSignatures>
        EmGenerator &bind_derived_class(const std::string &js_name = "");

        template <typename T> EmGenerator &bind_abstract_class(const std::string &js_name = "");

        template <typename Ret, typename... Args>
        EmGenerator &bind_function(const std::string &name, Ret (*func)(Args...));

        EmGenerator &add_utilities() ;

        static std::string get_typescript_declarations();
    };

    EmGenerator create_bindings() ;

} // namespace rosetta::em

// ============================================================================
// Macros
// ============================================================================

#define BEGIN_EM_MODULE(module_name)   \
    EMSCRIPTEN_BINDINGS(module_name) { \
        auto gen = rosetta::em::create_bindings();

#define END_EM_MODULE() }

#define BIND_EM_CLASS(Class) gen.bind_class<Class>(#Class);
#define BIND_EM_CLASS_AUTO(Class, ...) gen.bind_class<Class, __VA_ARGS__>(#Class);
#define BIND_EM_CLASS_FACTORY(Class, factory) gen.bind_class_with_factory<Class>(factory, #Class);
#define BIND_EM_DERIVED_CLASS(Derived, Base, ...) \
    gen.bind_derived_class<Derived, Base, __VA_ARGS__>(#Derived);
#define BIND_EM_ABSTRACT_CLASS(Class) gen.bind_abstract_class<Class>(#Class);

#define BIND_EM_FUNCTION(func) gen.bind_function(#func, func);

#define BIND_EM_UTILITIES() gen.add_utilities();

// Register vector type converter for custom classes
#define BIND_EM_VECTOR_TYPE(Class) rosetta::em::bind_vector_type<Class>();

/** Register function callback converter for std::function types Usage:
 * - `BIND_EM_FUNCTION_TYPE(Point,Point)` for `std::function<Point(Point)>`
 * - `BIND_EM_FUNCTION_TYPE(void, int, double)` for `std::function<void(int, double)>`
 */
#define BIND_EM_FUNCTION_TYPE(Ret, ...) rosetta::em::bind_function_type<Ret, __VA_ARGS__>();

#include "inline/em_generator.hxx"