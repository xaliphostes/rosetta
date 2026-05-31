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

#include <experimental/meta>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <rosetta/annotations.h>
#include <rosetta/docgen.h>
#include <rosetta/walk.h>
#include <string>
#include <string_view>
#include <utility>

namespace rosetta {

    /**
     * @brief Per-class binding configuration. Specialize this trait for each
     * type you want `rosetta::generate<T>` to scaffold bindings for. The
     * class itself stays unmodified; all of the metadata lives here.
     *
     * Required static members:
     *   - `targets` — iterable of `const char*` ("python", "node", "rest", "web")
     *   - `lib`     — `const char*` library / module name
     *   - `header`  — `const char*` basename used in `#include "..."`
     *
     * Example:
     *   template <> struct rosetta::binding_info<Person> {
     *       static constexpr std::array  targets{"python", "node"};
     *       static constexpr const char *lib    = "reflected_person";
     *       static constexpr const char *header = "person.h";
     *   };
     */
    template <class T> struct binding_info; // primary — must be specialized

    struct GenerateOptions {
        std::filesystem::path out_dir;         // root of the generated tree
        std::filesystem::path user_include;    // dir containing the class header
        std::filesystem::path rosetta_include; // path to rosetta's include/
    };

    /**
     * @brief Scaffold a per-backend binding project for T under opt.out_dir,
     * driven by the `rosetta::binding_info<T>` trait specialization.
     */
    template <typename T> void generate(const GenerateOptions &opt);

} // namespace rosetta

#include "inline/generate.hxx"
