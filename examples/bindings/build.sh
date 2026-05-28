#!/usr/bin/env bash
# Build one or more Rosetta backend examples.
#
# Usage:
#   ./build.sh                    # python + rest (defaults; pure cmake)
#   ./build.sh python rest        # explicit list
#   ./build.sh node               # adds N-API via npm + cmake-js
#   ./build.sh web                # adds emscripten/embind (needs emsdk activated)
#   ./build.sh all                # python rest node web
#   ./build.sh clean              # rm -rf each backend's build dir
#
# Env knobs:
#   GENERATOR=Ninja|Unix Makefiles|...     CMake generator (default: Ninja)
#   JOBS=<N>                                Parallel build jobs (default: nproc)
#   CLANG_P2996_ROOT=/path                  Override clang-p2996 fork location

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GENERATOR="${GENERATOR:-Ninja}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}"

# Forward an explicit clang-p2996 path to cmake / cmake-js when set.
CMAKE_EXTRA=()
CMAKEJS_EXTRA=()
if [[ -n "${CLANG_P2996_ROOT:-}" ]]; then
    CMAKE_EXTRA+=(-DCLANG_P2996_ROOT="${CLANG_P2996_ROOT}")
    CMAKEJS_EXTRA+=(--CDCLANG_P2996_ROOT="${CLANG_P2996_ROOT}")
fi

step() { printf '\n==> %s\n' "$1"; }

build_python() {
    step "python (pybind11)"
    cmake -G "$GENERATOR" -S "$SCRIPT_DIR/python" -B "$SCRIPT_DIR/python/build" \
          "${CMAKE_EXTRA[@]}"
    cmake --build "$SCRIPT_DIR/python/build" -j "$JOBS"
}

build_rest() {
    step "rest (httplib + json)"
    cmake -G "$GENERATOR" -S "$SCRIPT_DIR/rest" -B "$SCRIPT_DIR/rest/build" \
          "${CMAKE_EXTRA[@]}"
    cmake --build "$SCRIPT_DIR/rest/build" -j "$JOBS"
}

build_node() {
    step "node (N-API via cmake-js)"
    (
        cd "$SCRIPT_DIR/node"
        [[ -d node_modules ]] || npm install
        npx cmake-js compile --parallel "$JOBS" "${CMAKEJS_EXTRA[@]}"
    )
}

build_web() {
    step "web (emscripten)"
    if ! command -v emcmake >/dev/null 2>&1; then
        echo "error: emcmake not on PATH. Activate emsdk first:" >&2
        echo "       source <emsdk>/emsdk_env.sh" >&2
        exit 1
    fi
    emcmake cmake -G "$GENERATOR" -S "$SCRIPT_DIR/web" -B "$SCRIPT_DIR/web/build"
    cmake --build "$SCRIPT_DIR/web/build" -j "$JOBS"
}

do_clean() {
    step "clean"
    rm -rf "$SCRIPT_DIR/python/build" \
           "$SCRIPT_DIR/rest/build"   \
           "$SCRIPT_DIR/node/build"   \
           "$SCRIPT_DIR/web/build"
    echo "removed all build dirs"
}

# --- dispatch ---
backends=("$@")
if [[ ${#backends[@]} -eq 0 ]]; then
    backends=(python rest)
elif [[ ${#backends[@]} -eq 1 && "${backends[0]}" == "all" ]]; then
    backends=(python rest node web)
elif [[ ${#backends[@]} -eq 1 && "${backends[0]}" == "clean" ]]; then
    do_clean
    exit 0
fi

for b in "${backends[@]}"; do
    case "$b" in
        python) build_python ;;
        rest)   build_rest ;;
        node)   build_node ;;
        web)    build_web ;;
        *)      echo "error: unknown backend '$b' (valid: python rest node web all clean)" >&2
                exit 1 ;;
    esac
done

printf '\nBuilt: %s\n' "${backends[*]}"
