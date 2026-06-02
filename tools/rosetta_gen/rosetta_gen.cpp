// SPDX-FileCopyrightText: Copyright (c) fmaerten@gmail.com
// SPDX-License-Identifier: UNLICENSED

// Framework-level rosetta_gen tool. Reads manifest.json and emits a
// per-project tool source tree:
//
//   <out_dir>/bindings.h               rosetta::binding_info<T> specializations
//   <out_dir>/<generator_name>.cpp     main() with one generate<T> per class
//   <out_dir>/CMakeLists.txt           builds <generator_name>
//
// `<generator_name>` is the manifest-level `generator_name` field — the
// driver tool's name. The user compiles `<generator_name>` and runs it
// to emit the per-backend bindings.
//
// Manifest shape:
//   {
//     "user_include": "./geom",
//     "rosetta_include": "../../include",
//     "generator_name": "generator_geom",   // driver tool / CMake target
//     "module_name": "geom",                 // default binding module name
//     "targets": [                           // shared by every class
//       { "lang": "python", "name": "pygeom" },  // per-target module name
//       "node"                                   // shorthand: uses module_name
//     ],
//     "classes": [
//       { "name": "Model", "header": "Model.h" },
//       { "header": "Point.h" }              // name derived from header stem
//     ]
//   }
//
// One generator emits a single combined module per backend exposing
// every class. Each backend's module name is the target's `name` (or
// the manifest's `module_name` for a shorthand string target).

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using json   = nlohmann::json;

struct ClassEntry {
    std::string name;
    std::string header;
};

struct TargetEntry {
    std::string lang; // "python", "node", "rest", "web"
    std::string name; // module / library name for this backend
};

struct Manifest {
    fs::path                 user_include;    // absolute
    fs::path                 rosetta_include; // absolute
    std::string              generator_name;  // driver tool / CMake target name
    std::vector<TargetEntry> targets;         // backends + per-backend module name
    std::vector<ClassEntry>  classes;
    std::vector<std::string> plugins;         // extra .cpp sources (absolute) for the driver

    // CMake target / binary basename.
    std::string target() const { return generator_name; }
};

static Manifest load(const fs::path &manifest_path) {
    std::ifstream in(manifest_path);
    if (!in)
        throw std::runtime_error("cannot open " + manifest_path.string());
    // Tolerate // and /* */ comments in the manifest.
    json j = json::parse(in, /*cb=*/nullptr, /*allow_exceptions=*/true,
                         /*ignore_comments=*/true);

    Manifest       m;
    const fs::path base = fs::absolute(manifest_path).parent_path();

    m.user_include = fs::weakly_canonical(
        base / fs::path(j.at("user_include").get<std::string>()));
    m.rosetta_include = fs::weakly_canonical(
        base / fs::path(j.at("rosetta_include").get<std::string>()));

    // `generator_name` is optional; falls back to the manifest's parent
    // directory name (the driver tool / CMake target name).
    m.generator_name = j.contains("generator_name")
            ? j.at("generator_name").get<std::string>()
            : base.filename().string();

    // `module_name` is optional too; the default binding module name when a
    // target gives no `name`. Falls back to `generator_name`.
    const std::string module_name =
        j.contains("module_name") ? j.at("module_name").get<std::string>()
                                  : m.generator_name;

    // A target is either a bare string ("node") — using module_name — or an
    // object { "lang": ..., "name": ... } overriding the module name.
    for (const auto &t : j.at("targets")) {
        TargetEntry e;
        if (t.is_string()) {
            e.lang = t.get<std::string>();
            e.name = module_name;
        } else {
            e.lang = t.at("lang").get<std::string>();
            e.name = t.contains("name") ? t.at("name").get<std::string>() : module_name;
        }
        m.targets.push_back(std::move(e));
    }

    for (const auto &c : j.at("classes")) {
        ClassEntry e;
        e.header = c.at("header").get<std::string>();
        // `name` is optional; fall back to the header's basename (stem).
        e.name = c.contains("name") ? c.at("name").get<std::string>()
                                    : fs::path(e.header).stem().string();
        m.classes.push_back(std::move(e));
    }

    // `plugins` is optional: extra .cpp sources (e.g. a custom Backend +
    // BackendRegistrar) compiled into the driver. Resolved to absolute paths.
    if (j.contains("plugins"))
        for (const auto &p : j.at("plugins"))
            m.plugins.push_back(
                fs::weakly_canonical(base / fs::path(p.get<std::string>())).string());

    if (m.generator_name.empty())
        throw std::runtime_error(
            "cannot derive generator_name (set it explicitly in the manifest)");
    if (m.targets.empty())
        throw std::runtime_error("manifest has no targets");
    if (m.classes.empty())
        throw std::runtime_error("manifest has no class entries");

    return m;
}

static void write_file(const fs::path &p, const std::string &content) {
    fs::create_directories(p.parent_path());
    std::ofstream(p) << content;
}

static std::string render_bindings_h(const Manifest &m) {
    std::ostringstream out;
    out << "// Generated by rosetta_gen — do not edit by hand.\n"
        << "#pragma once\n\n"
        << "#include <rosetta/generate.h>\n";
    for (const auto &c : m.classes)
        out << "#include \"" << c.header << "\"\n";
    out << "\n";
    for (const auto &c : m.classes) {
        out << "template <> struct rosetta::binding_info<" << c.name << "> {\n"
            << "    static constexpr const char *header = \"" << c.header << "\";\n"
            << "};\n\n";
    }
    return out.str();
}

static std::string render_project_gen_cpp(const Manifest &m) {
    const std::string target = m.target();
    std::ostringstream out;
    out << "// Generated by rosetta_gen — do not edit by hand.\n"
        << "#include \"bindings.h\"\n"
        << "#include <cstdio>\n"
        << "#include <cstring>\n\n"
        << "namespace {\n"
        << "    const char *arg(int argc, char **argv, const char *flag) {\n"
        << "        for (int i = 1; i + 1 < argc; ++i)\n"
        << "            if (std::strcmp(argv[i], flag) == 0) return argv[i + 1];\n"
        << "        return nullptr;\n"
        << "    }\n"
        << "}\n\n"
        << "int main(int argc, char **argv) {\n"
        << "    rosetta::GenerateOptions opt;\n"
        << "    if (const char *v = arg(argc, argv, \"--out\")) opt.out_dir = v;\n"
        << "    opt.user_include    = \"" << m.user_include.string() << "\";\n"
        << "    opt.rosetta_include = \"" << m.rosetta_include.string() << "\";\n"
        << "    opt.targets         = {\n";
    for (const auto &t : m.targets)
        out << "        {\"" << t.lang << "\", \"" << t.name << "\"},\n";
    out << "    };\n\n"
        << "    if (opt.out_dir.empty()) {\n"
        << "        std::fprintf(stderr, \"usage: " << target << " --out <dir>\\n\");\n"
        << "        return 1;\n"
        << "    }\n\n";
    out << "    rosetta::generate<";
    for (std::size_t i = 0; i < m.classes.size(); ++i)
        out << (i ? ", " : "") << m.classes[i].name;
    out << ">(opt);\n";
    out << "\n    std::fprintf(stderr, \"wrote scaffolding to %s\\n\",\n"
        << "                 opt.out_dir.string().c_str());\n"
        << "    return 0;\n"
        << "}\n";
    return out.str();
}

static std::string render_cmakelists(const Manifest &m) {
    const std::string target = "generator"; //m.target();
    std::ostringstream out;
    out << "# Generated by rosetta_gen — do not edit by hand.\n"
        << "cmake_minimum_required(VERSION 3.28)\n\n"
        << "set(CLANG_P2996_ROOT \"$ENV{HOME}/devs/c++/clang-p2996/build\"\n"
        << "    CACHE PATH \"Bloomberg clang-p2996 fork build directory\")\n"
        << "if(NOT CMAKE_CXX_COMPILER)\n"
        << "    set(CMAKE_C_COMPILER   \"${CLANG_P2996_ROOT}/bin/clang\")\n"
        << "    set(CMAKE_CXX_COMPILER \"${CLANG_P2996_ROOT}/bin/clang++\")\n"
        << "endif()\n\n"
        << "project(" << target << " CXX)\n\n"
        << "set(CMAKE_CXX_STANDARD 26)\n"
        << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
        << "set(CMAKE_CXX_EXTENSIONS OFF)\n"
        << "set(CMAKE_CXX_SCAN_FOR_MODULES OFF)\n\n"
        << "# Place the built binary in the parent folder (where rosetta_gen\n"
        << "# was invoked), not in this generation folder's build tree.\n"
        << "set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..)\n\n";

    // add_executable with the driver source plus any plugin sources.
    out << "add_executable(" << target << " " << m.target() << ".cpp";
    for (const auto &p : m.plugins)
        out << "\n    " << p;
    out << ")\n\n";

    out << "target_include_directories(" << target << " PRIVATE\n"
        << "    " << m.user_include.string() << "\n"
        << "    " << m.rosetta_include.string() << ")\n\n"
        << "target_compile_options(" << target << " PRIVATE\n"
        << "    -freflection -freflection-latest -fexperimental-library -fannotation-attributes)\n\n"
        << "target_link_options(" << target << " PRIVATE\n"
        << "    -nostdlib++ -L${CLANG_P2996_ROOT}/lib -Wl,-rpath,${CLANG_P2996_ROOT}/lib\n"
        << "    -lc++ -lc++abi)\n";
    return out.str();
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        std::fprintf(stderr, "usage: rosetta_gen <manifest.json> [out_dir]\n");
        return 1;
    }
    const fs::path manifest_path = argv[1];
    const fs::path out_dir       = (argc == 3)
            ? fs::path(argv[2])
            : fs::absolute(manifest_path).parent_path() / "generated";

    try {
        const Manifest    m      = load(manifest_path);
        const std::string target = m.target();
        write_file(out_dir / "bindings.h", render_bindings_h(m));
        write_file(out_dir / (target + ".cpp"), render_project_gen_cpp(m));
        write_file(out_dir / "CMakeLists.txt", render_cmakelists(m));

        std::fprintf(stderr,
                     "wrote %s/{bindings.h, %s.cpp, CMakeLists.txt}\n",
                     out_dir.string().c_str(),
                     target.c_str());
    } catch (const std::exception &e) {
        std::fprintf(stderr, "rosetta_gen: %s\n", e.what());
        return 1;
    }

    return 0;
}
