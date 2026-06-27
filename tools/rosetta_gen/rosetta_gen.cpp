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
//     "generator_name": "generator_geom",           // driver tool / CMake target
//     "module_name": "geom",                        // default binding module name
//     "cpp26_root": "$ENV{HOME}/clang-p2996/build", // optional: C++26/P2996
//                                                   // toolchain root for the thin
//                                                   // (reflection) backends. Default:
//                                                   // $ENV{HOME}/devs/c++/clang-p2996/build
//     "cpp26_cxx": "clang++",                       // optional: C++ compiler (name or
//                                                   //   path). Default ${root}/bin/clang++
//     "cpp26_cc":  "clang",                         // optional: C compiler.
//                                                   //   Default ${root}/bin/clang
//     "cpp26_lib": "/path/to/fork/lib",             // optional: dir with libc++/
//                                                   //   libc++abi (-L/-rpath).
//                                                   //   Default ${root}/lib
//     "qt_dir": "$ENV{HOME}/Qt/6.8.3/macos",        // optional: Qt 6 prefix for the
//                                                   //   qt-expanded / qml-expanded
//                                                   //   backends. Default that path.
//     "user_lib": {                                 // optional: external library to link
//       "name": "space",                            //   the bindings against (-lspace).
//       "dir":  "../space/bin"                       //   Use when the bound headers only
//     },                                            //   declare the API and the bodies
//                                                   //   live in a separately-compiled
//                                                   //   shared/static lib. Path is relative
//                                                   //   to the manifest.
//     "targets": [                                  // shared by every class
//       { "lang": "python", "name": "pygeom" },     // per-target module name
//       "node"                                      // shorthand: uses module_name
//     ],
//     "classes": [
//       { "name": "Model", "header": "Model.h",
//         "annotations": "Model.ann.json" },        // optional out-of-line annotations
//       { "header": "Point.h" }                     // name derived from header stem
//     ],
//     "functions": [                                // optional: free (non-member) fns
//       { "name": "transform", "header": "common.h", "doc": "..." }
//     ]                                             // name may be qualified (api::add)
//   }
//
// One generator emits a single combined module per backend exposing
// every class. Each backend's module name is the target's `name` (or
// the manifest's `module_name` for a shorthand string target).

#include <algorithm>
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
    fs::path    annotations; // optional out-of-line annotation JSON (absolute); empty if none
};

struct FunctionEntry {
    std::string name;   // (optionally qualified) C++ function name, e.g. "api::add"
    std::string header; // header declaring it
    std::string doc;    // optional manifest doc string
};

struct TargetEntry {
    std::string lang; // "python", "node", "rest", "web"
    std::string name; // module / library name for this backend
};

struct Manifest {
    fs::path                   user_include;    // absolute
    fs::path                   rosetta_include; // absolute
    std::string                generator_name;  // driver tool / CMake target name
    std::vector<TargetEntry>   targets;         // backends + per-backend module name
    std::vector<ClassEntry>    classes;
    std::vector<FunctionEntry> functions; // free functions to expose
    std::vector<std::string>   plugins;   // extra .cpp sources (absolute) for the driver
    std::string                cpp26_root; // optional C++26/P2996 toolchain root (verbatim)
    std::string                cpp26_cxx;  // optional C++ compiler (name or path)
    std::string                cpp26_cc;   // optional C compiler (name or path)
    std::string                cpp26_lib;  // optional fork stdlib dir (libc++/libc++abi)
    std::string                qt_dir;     // optional Qt 6 prefix (qt/qml backends)
    std::string                user_lib_name; // optional external lib to link bindings against
    std::string                user_lib_dir;  // optional dir holding it (absolute; -L / rpath)

    // CMake target / binary basename.
    std::string target() const { return generator_name; }
};

// Defaults baked into the driver CMakeLists when the manifest omits a field.
// Kept in sync with rosetta::gen_detail::DEFAULT_CPP26_*. The compiler / stdlib
// defaults are CMake expressions deriving from CLANG_P2996_ROOT, so setting only
// "cpp26_root" moves all three together.
static const char *kDefaultCpp26Root = "$ENV{HOME}/devs/c++/clang-p2996/build";
static const char *kDefaultCpp26Cxx  = "${CLANG_P2996_ROOT}/bin/clang++";
static const char *kDefaultCpp26Cc   = "${CLANG_P2996_ROOT}/bin/clang";
static const char *kDefaultCpp26Lib  = "${CLANG_P2996_ROOT}/lib";

static Manifest load(const fs::path &manifest_path) {
    std::ifstream in(manifest_path);
    if (!in) {
        throw std::runtime_error("cannot open " + manifest_path.string());
    }
    // Tolerate // and /* */ comments in the manifest.
    json j = json::parse(in, /*cb=*/nullptr, /*allow_exceptions=*/true,
                         /*ignore_comments=*/true);

    Manifest       m;
    const fs::path base = fs::absolute(manifest_path).parent_path();

    m.user_include = fs::weakly_canonical(base / fs::path(j.at("user_include").get<std::string>()));
    m.rosetta_include =
        fs::weakly_canonical(base / fs::path(j.at("rosetta_include").get<std::string>()));

    // `generator_name` is optional; falls back to the manifest's parent
    // directory name (the driver tool / CMake target name).
    m.generator_name = j.contains("generator_name") ? j.at("generator_name").get<std::string>()
                                                    : base.filename().string();

    // `module_name` is optional too; the default binding module name when a
    // target gives no `name`. Falls back to `generator_name`.
    const std::string module_name =
        j.contains("module_name") ? j.at("module_name").get<std::string>() : m.generator_name;

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
        // `annotations` is optional: an out-of-line annotation JSON side-car
        // (doc/range/readonly/combobox keyed by member name). Baked into
        // bindings.h at generation time, so the user's header stays clean.
        if (c.contains("annotations")) {
            e.annotations =
                fs::weakly_canonical(base / fs::path(c.at("annotations").get<std::string>()));
        }
        m.classes.push_back(std::move(e));
    }

    // `functions` is optional: free (non-member) functions to bind. Each entry
    // gives the (optionally qualified) function name and its declaring header;
    // `doc` is an optional description (free functions carry no in-source
    // annotation, keeping the user's headers untouched).
    if (j.contains("functions")) {
        for (const auto &f : j.at("functions")) {
            FunctionEntry e;
            e.name   = f.at("name").get<std::string>();
            e.header = f.at("header").get<std::string>();
            e.doc    = f.contains("doc") ? f.at("doc").get<std::string>() : std::string{};
            m.functions.push_back(std::move(e));
        }
    }

    // `plugins` is optional: extra .cpp sources (e.g. a custom Backend +
    // BackendRegistrar) compiled into the driver. Resolved to absolute paths.
    if (j.contains("plugins")) {
        for (const auto &p : j.at("plugins")) {
            m.plugins.push_back(
                fs::weakly_canonical(base / fs::path(p.get<std::string>())).string());
        }
    }

    // `cpp26_root` is optional: the path to the C++26 / P2996 reflection
    // toolchain root (the clang-p2996 build dir, holding bin/clang++ and lib/).
    // Stored verbatim so a value like "$ENV{HOME}/..." or an absolute path is
    // baked straight into the generated CMakeLists. Only the reflection-driven
    // (thin) targets use it; the stock *-expanded targets ignore it.
    if (j.contains("cpp26_root")) {
        m.cpp26_root = j.at("cpp26_root").get<std::string>();
    }
    // Optional finer-grained overrides; each defaults from cpp26_root if unset.
    //   cpp26_cxx — C++ compiler (name or path)   cpp26_cc — C compiler
    //   cpp26_lib — fork stdlib dir (libc++/libc++abi) for -L / -rpath
    if (j.contains("cpp26_cxx")) {
        m.cpp26_cxx = j.at("cpp26_cxx").get<std::string>();
    }
    if (j.contains("cpp26_cc")) {
        m.cpp26_cc = j.at("cpp26_cc").get<std::string>();
    }
    if (j.contains("cpp26_lib")) {
        m.cpp26_lib = j.at("cpp26_lib").get<std::string>();
    }
    // Optional Qt 6 install prefix for the qt-expanded / qml-expanded backends.
    if (j.contains("qt_dir")) {
        m.qt_dir = j.at("qt_dir").get<std::string>();
    }

    // `user_lib` is optional: an external library the generated bindings link
    // against. Use it when the bound headers only *declare* the API and its
    // bodies are compiled into a separate shared/static library. `name` is the
    // library base name (-l<name>); `dir` is the directory holding it, resolved
    // to an absolute path (relative to the manifest) for -L / rpath.
    if (j.contains("user_lib")) {
        const auto &ul = j.at("user_lib");
        m.user_lib_name = ul.at("name").get<std::string>();
        m.user_lib_dir =
            fs::weakly_canonical(base / fs::path(ul.at("dir").get<std::string>())).string();
    }

    if (m.generator_name.empty()) {
        throw std::runtime_error(
            "cannot derive generator_name (set it explicitly in the manifest)");
    }
    if (m.targets.empty()) {
        throw std::runtime_error("manifest has no targets");
    }
    if (m.classes.empty()) {
        throw std::runtime_error("manifest has no class entries");
    }

    return m;
}

static void write_file(const fs::path &p, const std::string &content) {
    fs::create_directories(p.parent_path());
    std::ofstream(p) << content;
}

static std::string read_file(const fs::path &p) {
    std::ifstream in(p, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open annotations file " + p.string());
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// JSON bytes -> "char(0x7b), char(0x0a), ..." for a std::to_array<char> literal.
// The explicit char() cast avoids a narrowing error for bytes >= 0x80 (UTF-8).
static std::string render_byte_array(const std::string &data) {
    std::ostringstream out;
    for (unsigned char ch : data) {
        char buf[16];
        std::snprintf(buf, sizeof buf, "char(0x%02x), ", ch);
        out << buf;
    }
    return out.str();
}

static std::string render_bindings_h(const Manifest &m) {
    std::ostringstream out;
    out << "// Generated by rosetta_gen — do not edit by hand.\n"
        << "#pragma once\n\n"
        << "#include <rosetta/generate.h>\n"
        << "#include <rosetta/annotate.h>\n";
    std::vector<std::string> seen;
    auto                     include_once = [&](const std::string &h) {
        if (std::find(seen.begin(), seen.end(), h) == seen.end()) {
            seen.push_back(h);
            out << "#include \"" << h << "\"\n";
        }
    };
    for (const auto &c : m.classes) {
        include_once(c.header);
    }
    for (const auto &f : m.functions) {
        include_once(f.header);
    }
    out << "\n";
    for (const auto &c : m.classes) {
        out << "template <> struct rosetta::binding_info<" << c.name << "> {\n"
            << "    static constexpr const char *header = \"" << c.header << "\";\n"
            << "};\n\n";
    }
    // Out-of-line annotations: bake each side-car JSON into an
    // ann_json_source<T> specialization. This TU includes the class header
    // (T is complete) before these appear, so the staleness static_assert and
    // the walk()-time merge both see them.
    for (const auto &c : m.classes) {
        if (c.annotations.empty()) {
            continue;
        }
        const std::string bytes = render_byte_array(read_file(c.annotations));
        out << "// out-of-line annotations for " << c.name << " <- "
            << c.annotations.filename().string() << "\n"
            << "template <> inline constexpr auto rosetta::detail::ann_storage<" << c.name
            << "> =\n    std::to_array<char>({" << bytes << "'\\0'});\n"
            << "template <> inline constexpr std::string_view rosetta::ann_json_source<" << c.name
            << "> =\n    std::string_view{rosetta::detail::ann_storage<" << c.name
            << ">.data(),\n                     rosetta::detail::ann_storage<" << c.name
            << ">.size() - 1};\n"
            << "static_assert(rosetta::detail::ann_keys_error<" << c.name
            << ">().empty(),\n              rosetta::detail::ann_keys_error<" << c.name
            << ">());\n\n";
    }
    return out.str();
}

static std::string render_project_gen_cpp(const Manifest &m) {
    const std::string  target = m.target();
    std::ostringstream out;
    out << "// Generated by rosetta_gen — do not edit by hand.\n"
        << "#include \"bindings.h\"\n"
        << "#include <cstdio>\n\n"
        << "int main(int argc, char **argv) {\n"
        << "    if (argc < 2) {\n"
        << "        std::fprintf(stderr, \"usage: " << target << " <out_dir>\\n\");\n"
        << "        return 1;\n"
        << "    }\n\n"
        << "    rosetta::GenerateOptions opt;\n"
        << "    opt.out_dir         = argv[1];\n"
        << "    opt.user_include    = \"" << m.user_include.string() << "\";\n"
        << "    opt.rosetta_include = \"" << m.rosetta_include.string() << "\";\n";
    // These reach the per-backend (thin) CMakeLists via GenContext; each is
    // emitted only when set (empty ⇒ generate() applies its built-in default).
    if (!m.cpp26_root.empty()) {
        out << "    opt.cpp26_root      = \"" << m.cpp26_root << "\";\n";
    }
    if (!m.cpp26_cxx.empty()) {
        out << "    opt.cpp26_cxx       = \"" << m.cpp26_cxx << "\";\n";
    }
    if (!m.cpp26_cc.empty()) {
        out << "    opt.cpp26_cc        = \"" << m.cpp26_cc << "\";\n";
    }
    if (!m.cpp26_lib.empty()) {
        out << "    opt.cpp26_lib       = \"" << m.cpp26_lib << "\";\n";
    }
    if (!m.qt_dir.empty()) {
        out << "    opt.qt_dir          = \"" << m.qt_dir << "\";\n";
    }
    // External library to link the bindings against (manifest "user_lib"). Only
    // the stock *-expanded backends consume these (see GenerateOptions).
    if (!m.user_lib_name.empty()) {
        out << "    opt.user_lib_name   = \"" << m.user_lib_name << "\";\n";
        out << "    opt.user_lib_dir    = \"" << m.user_lib_dir << "\";\n";
    }
    out << "    opt.targets         = {\n";
    for (const auto &t : m.targets) {
        out << "        {\"" << t.lang << "\", \"" << t.name << "\"},\n";
    }
    out << "    };\n";
    if (!m.functions.empty()) {
        out << "    opt.functions       = {\n";
        for (const auto &f : m.functions) {
            out << "        rosetta::make_function<^^" << f.name << ">(\"" << f.name << "\", \""
                << f.header << "\", \"" << f.doc << "\"),\n";
        }
        out << "    };\n";
    }
    out << "\n";
    out << "    rosetta::generate<";
    for (std::size_t i = 0; i < m.classes.size(); ++i) {
        out << (i ? ", " : "") << m.classes[i].name;
    }
    out << ">(opt);\n";
    out << "\n    std::fprintf(stderr, \"wrote scaffolding to %s\\n\",\n"
        << "                 opt.out_dir.string().c_str());\n"
        << "    return 0;\n"
        << "}\n";
    return out.str();
}

static std::string render_cmakelists(const Manifest &m) {
    const std::string  target = "generator"; // m.target();
    std::ostringstream out;
    const std::string cpp26_root = m.cpp26_root.empty() ? kDefaultCpp26Root : m.cpp26_root;
    const std::string cpp26_cxx  = m.cpp26_cxx.empty() ? kDefaultCpp26Cxx : m.cpp26_cxx;
    const std::string cpp26_cc   = m.cpp26_cc.empty() ? kDefaultCpp26Cc : m.cpp26_cc;
    const std::string cpp26_lib  = m.cpp26_lib.empty() ? kDefaultCpp26Lib : m.cpp26_lib;
    out << "# Generated by rosetta_gen — do not edit by hand.\n"
        << "cmake_minimum_required(VERSION 3.28)\n\n"
        << "set(CLANG_P2996_ROOT \"" << cpp26_root << "\"\n"
        << "    CACHE PATH \"C++26 / P2996 reflection toolchain root (clang-p2996 build dir)\")\n"
        << "set(ROSETTA_CXX_COMPILER \"" << cpp26_cxx << "\"\n"
        << "    CACHE FILEPATH \"C++26 / P2996 C++ compiler\")\n"
        << "set(ROSETTA_C_COMPILER \"" << cpp26_cc << "\"\n"
        << "    CACHE FILEPATH \"C++26 / P2996 C compiler\")\n"
        << "set(ROSETTA_STDLIB \"" << cpp26_lib << "\"\n"
        << "    CACHE PATH \"Directory holding the fork's libc++ / libc++abi (-L and -rpath)\")\n"
        << "if(NOT CMAKE_CXX_COMPILER)\n"
        << "    set(CMAKE_C_COMPILER   \"${ROSETTA_C_COMPILER}\")\n"
        << "    set(CMAKE_CXX_COMPILER \"${ROSETTA_CXX_COMPILER}\")\n"
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
    for (const auto &p : m.plugins) {
        out << "\n    " << p;
    }
    out << ")\n\n";

    out << "target_include_directories(" << target << " PRIVATE\n"
        << "    " << m.user_include.string() << "\n"
        << "    " << m.rosetta_include.string() << ")\n\n"
        << "target_compile_options(" << target << " PRIVATE\n"
        << "    -freflection -freflection-latest -fexperimental-library "
           "-fannotation-attributes)\n\n"
        << "target_link_options(" << target << " PRIVATE\n"
        << "    -nostdlib++ -L${ROSETTA_STDLIB} -Wl,-rpath,${ROSETTA_STDLIB}\n"
        << "    -lc++ -lc++abi)\n";

    // If the bound headers only declare the API (bodies in an external library),
    // the driver links against it too: the reflection walk instantiates each
    // bound type (e.g. to read default-constructed field values), so it needs
    // the out-of-line definitions at link AND run time (hence rpath).
    if (!m.user_lib_name.empty()) {
        out << "\n# External user library (manifest \"user_lib\").\n"
            << "target_link_directories(" << target << " PRIVATE " << m.user_lib_dir << ")\n"
            << "target_link_libraries(" << target << " PRIVATE " << m.user_lib_name << ")\n"
            << "set_target_properties(" << target << " PROPERTIES\n"
            << "    BUILD_RPATH \"" << m.user_lib_dir << "\")\n";
    }
    return out.str();
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        std::fprintf(stderr, "usage: rosetta_gen <manifest.json> [out_dir]\n");
        return 1;
    }
    const fs::path manifest_path = argv[1];
    const fs::path out_dir =
        (argc == 3) ? fs::path(argv[2]) : fs::absolute(manifest_path).parent_path() / "generated";

    try {
        const Manifest    m      = load(manifest_path);
        const std::string target = m.target();
        write_file(out_dir / "bindings.h", render_bindings_h(m));
        write_file(out_dir / (target + ".cpp"), render_project_gen_cpp(m));
        write_file(out_dir / "CMakeLists.txt", render_cmakelists(m));

        std::fprintf(stderr, "wrote %s/{bindings.h, %s.cpp, CMakeLists.txt}\n",
                     out_dir.string().c_str(), target.c_str());
    } catch (const std::exception &e) {
        std::fprintf(stderr, "rosetta_gen: %s\n", e.what());
        return 1;
    }

    return 0;
}
