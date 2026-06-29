# Changelog

All notable changes to **rosetta** are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
This project has not yet cut a tagged release, so entries are grouped by date
rather than by version number. Dates are `YYYY-MM-DD`.

## [Unreleased]

### Added
- **`user_sources` manifest field** — a list of user `.cpp` files compiled
  directly into every generated binding target via `target_sources(...)`. Use
  it when the bound headers only *declare* the API and you want the bodies
  built into the binding rather than linked from a pre-built `user_lib`. Works
  alongside or instead of `user_lib`; ignored by the text-only backends.
  Entries may be **shell globs** (e.g. `src/algorithms/*.cpp`), expanded at
  generation time; results are sorted and de-duplicated.
- **Multiple include directories** — `user_include` now accepts an array of
  directories (a class `header` is searched across all of them, like a
  compiler's `-I` order), in addition to the existing single-string form.

### Fixed
- **Node: reference parameters of bound types** — the N-API backend materialized
  every argument by value (`from_napi<remove_cvref_t<P>>`), so a non-const
  lvalue-reference parameter couldn't bind and any function taking a bound type
  by reference (e.g. an algorithm taking `SurfaceMesh&` and mutating it in place)
  failed to compile. A new `arg_from_napi<P>` binds reference parameters of
  wrapped user types directly to the wrapper's persistent `inner` object, so
  in-place mutations propagate back to the JS object (and `const T&` no longer
  copies). Applied to free functions, instance/static methods, and constructors.
- **Node: unbindable free functions are skipped, not fatal** — a free function
  whose signature can't cross the N-API boundary (a pointer out-parameter such as
  `std::vector<T>*`, a `std::function`, an unsupported return type, …) previously
  broke the whole module build. `bind_napi_function` now guards on a consteval
  `napi_free_supported<F>()` and binds a stub that throws on call instead, so the
  module still loads and every other function stays usable.
- **Template-specialization type names** — `class_name<T>()` no longer hard-errors
  when a bound free function takes or returns a template specialization
  (e.g. `pmp::Matrix<float, 3, 1>`, `Eigen::SparseMatrix<double>`); it falls
  back to the full display spelling when the type has no plain identifier.
- **Operator / conversion members** — the member walk skips functions without an
  identifier (`operator==`, `operator[]`, `operator T`), which can't be bound by
  name to a target language and previously broke generation for such classes.

### Docs
- Documented `user_sources`, multiple `user_include` directories, and added an
  initial `docs/MANIFEST.md` reference for the manifest file.
- Added this `CHANGELOG.md`.

## 2026-06-27

### Added
- External third-party library linking: full example using both dynamic and
  static linkage, with handling for libraries that live in their own namespace.

### Changed
- Reworked dynamic-vs-static user-library linkage; namespaced third-party
  libraries now bind without qualifying every spelling.

## 2026-06-26

### Added
- **Java backend and visitor** — C-ABI shared library plus handle-backed FFM
  wrappers.
- More backend examples wired up for the `geom-lib` example.

### Docs
- README updates, including the backend capability table.

## 2026-06-25

### Added
- **C# backend** — C-ABI shared library plus P/Invoke wrappers, buildable
  without a C++26 toolchain (expanded path).

## 2026-06-20 – 2026-06-21

### Added
- **nanobind** visitor/backend support, plus a `nanobind-expanded` variant.
- **Expanded backends** — reflection runs once on a C++26 host and the generated
  sources build with a stock compiler, for toolchains that don't yet support
  C++26 / P2996 reflection.
- Browser example for the WebAssembly target.

### Changed
- "Transparent rosetta" pass over the generation flow.

## 2026-06-16 – 2026-06-19

### Added
- **Inheritance introspection** — base-class flattening, `virtual_spec` carried
  through the walk, and trampolines for pybind11 and Node so virtual/overriding
  methods are distinguished from plain ones.
- **ParaView** Server Manager plugin XML generation.

### Changed
- Refactored doc generation.

### Docs
- README updates.

## 2026-06-12 – 2026-06-14

### Added
- **Out-of-line (external file) annotations** — annotate bound types from a JSON
  side-car so the headers stay clean.
- GoogleTest integration and `signal::scoped_connect`.

### Changed
- Reorganized sources; simplified the mini-MOC signal handling and refactored
  the mini-MOC.
- Moved the reflection walk into an inline `walk.hxx`.

### Removed
- Obsolete files.

## 2026-06-09 – 2026-06-11

### Added
- **Julia** language binding/backend (CxxWrap.jl / jlcxx).

### Changed
- Moved the Qt/QML inspectors under `include/rosetta`; general reorganization and
  a linter pass.

### Docs
- README and Julia example updates.

## 2026-06-02 – 2026-06-08

### Added
- **Enum** support.
- **Free (non-member) function** binding.
- **REST** backend (cpp-httplib JSON server + browser client) and a JSON
  (de)serializer.
- **OpenAPI 3.1** spec backend.
- **TypeScript** (`.d.ts`) and **Markdown** documentation backends.

### Changed
- Split backends into separate files; renamed the `web` target to `wasm`.
- Inlined the doc generator.

## 2026-05-31 – 2026-06-01

### Added
- The `rosetta_gen` manifest-driven project generator.
- Constructor binding support.

## 2026-05-29 – 2026-05-30

### Added
- CLI tools for generating skeletons.
- Richer annotations and an initial Qt/QML example.

### Docs
- Early documentation.

## 2026-05-28

### Added
- Initial commit: the rosetta framework and introductory slides.

[Unreleased]: https://github.com/Xaliphostes/rosetta/compare/df8960d...HEAD
