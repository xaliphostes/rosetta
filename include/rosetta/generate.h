// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Reflection-driven binding scaffolder. `rosetta::generate<T>(...)` reads
// the `rosetta::binding_info<T>` trait specialization and emits a
// per-backend project tree under <out_dir>:
//
//   <out_dir>/python/{auto_pybind.cpp, CMakeLists.txt, README.md}
//   <out_dir>/node/  {auto_napi.cpp,   CMakeLists.txt, package.json, README.md}
//   <out_dir>/rest/  {auto_rest.cpp,   CMakeLists.txt, README.md}
//   <out_dir>/web/   {auto_emscripten.cpp, CMakeLists.txt, README.md}
//
// The trait carries the per-class config (target list, lib name, header
// basename) so the class definition stays pristine. Example:
//
//   template <> struct rosetta::binding_info<Person> {
//       static constexpr std::array  targets{"python", "node"};
//       static constexpr const char *lib    = "reflected_person";
//       static constexpr const char *header = "person.h";
//   };
//
// The user-include and rosetta-include directory paths are not in the
// trait — they are build-context-dependent and come from the caller
// (typically CLI flags on the driver).

#pragma once

#include <cstdio>
#include <experimental/meta>
#include <filesystem>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <rosetta/annotations.h>
#include <rosetta/docgen.h>
#include <rosetta/walk.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace rosetta {

    /**
     * @brief Per-class binding configuration. Specialize this trait for each
     * type you pass to `rosetta::generate<Ts...>`. The class itself stays
     * unmodified; the only per-class metadata is its header basename.
     *
     * Required static member:
     *   - `header` — `const char*` basename used in `#include "..."`
     *
     * The module / library name and the target backends are no longer
     * per-class — they live in `GenerateOptions` because one generator
     * call emits a single combined module per backend exposing every
     * class. Example:
     *   template <> struct rosetta::binding_info<Person> {
     *       static constexpr const char *header = "person.h";
     *   };
     */
    template <class T> struct binding_info; // primary — must be specialized

    /**
     * @brief One output target: a backend language and the module /
     * library name to bake into that backend's generated bindings.
     */
    struct TargetSpec {
        std::string lang; // "python", "node", "rest", "web"
        std::string name; // module / library name for this backend
    };

    struct GenerateOptions {
        std::filesystem::path   out_dir;         // root of the generated tree
        std::filesystem::path   user_include;    // dir containing the class headers
        std::filesystem::path   rosetta_include; // path to rosetta's include/
        std::vector<TargetSpec> targets;         // backends + per-backend module name
    };

    /**
     * @brief A reflected type, reduced to a small language-neutral descriptor
     * so pure-data backends (TypeScript, JSON Schema, …) can render it without
     * reflection. `kind` is one of: "number", "boolean", "string", "void",
     * "vector", "object", "unknown". For "vector", `element` holds one entry
     * (the element type); for "object", `object` is the class identifier.
     */
    struct GenType {
        std::string          kind = "unknown";
        std::string          object;  // class identifier when kind == "object"
        std::vector<GenType> element; // 0 or 1 entry, the element when "vector"
    };

    struct GenField {
        std::string name;
        GenType     type;
        bool        is_readonly = false;
        std::string doc; // rosetta::doc annotation text, if any
    };

    struct GenParam {
        std::string name; // synthesized "argN" (parameter names aren't reflected)
        GenType     type;
    };

    struct GenMethod {
        std::string            name;
        bool                   is_static = false;
        GenType                ret;
        std::vector<GenParam>  params;
        std::string            doc; // rosetta::doc annotation text, if any
    };

    /**
     * @brief One class, erased to the plain data a backend needs. `generate`
     * fills this up front (the only place reflection runs), so backends are
     * pure text templating. Member type info (`fields` / `methods` / `ctors`)
     * is populated for pure-data backends; backends that emit C++ and defer to
     * a runtime visitor can ignore it and use just `name` / `header`.
     */
    struct GenClass {
        std::string name;   // reflected C++ identifier
        std::string header; // binding_info<T>::header — basename for #include
        std::string doc;    // generate_markdown<T>() — README body fragment

        std::vector<GenField>              fields;  // public data members
        std::vector<GenMethod>             methods; // instance + static methods
        std::vector<std::vector<GenParam>> ctors;   // one param list per constructor
    };

    /**
     * @brief Everything a backend needs to emit one target's project tree.
     */
    struct GenContext {
        std::filesystem::path out_dir;         // root of the generated tree
        std::string           lib;             // this target's module / library name
        std::vector<GenClass> classes;         // all classes to expose
        std::string           user_include;    // dir containing the class headers
        std::string           rosetta_include; // path to rosetta's include/
    };

    /**
     * @brief Code-generation backend for one target language. Implement this
     * and register it (see `register_backend`) to teach `generate` a new
     * backend — no edit to `generate` itself is required.
     */
    struct Backend {
        virtual ~Backend()                       = default;
        virtual void emit(const GenContext &) const = 0;
    };

    /**
     * @brief The lang → backend map consulted by `generate` at run time.
     * Seeded with the built-in "python", "node", "rest", "web" backends on
     * first use.
     */
    std::map<std::string, std::shared_ptr<Backend>> &backend_registry();

    /** @brief Register (or override) the backend handling `lang`. */
    void register_backend(std::string lang, std::shared_ptr<Backend> backend);

    /**
     * @brief Static-init helper: declare one at namespace scope in a plugin
     * translation unit linked into the generator to register a backend before
     * `main` runs. e.g.
     *   static rosetta::BackendRegistrar lua{"lua", std::make_shared<LuaBackend>()};
     */
    struct BackendRegistrar {
        BackendRegistrar(std::string lang, std::shared_ptr<Backend> backend) {
            register_backend(std::move(lang), std::move(backend));
        }
    };

    /**
     * @brief Scaffold the per-backend binding projects under opt.out_dir for
     * the whole set of classes `Ts...`. The pack is erased into a
     * `std::vector<GenClass>` and each target is dispatched through
     * `backend_registry()`; this function never changes when a backend is
     * added. Per-class headers come from the `rosetta::binding_info<T>` trait.
     */
    template <typename... Ts> void generate(const GenerateOptions &opt);

} // namespace rosetta

#include "inline/generate.hxx"
