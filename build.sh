#!/bin/bash
set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"
requested_plugin="${1:-}"
build_generator="Unix Makefiles"

if command -v ninja >/dev/null 2>&1; then
    build_generator="Ninja"
fi

resolve_plugin_target() {
    local request="$1"
    local normalized_request="${request#src/}"
    local plugin_cmake="${script_dir}/src/${normalized_request}/CMakeLists.txt"
    local detected_target=""

    if [[ "${request}" == *"-vst3" ]]; then
        printf '%s' "${request}"
        return
    fi

    if [[ -f "${plugin_cmake}" ]]; then
        detected_target="$(sed -nE 's/^[[:space:]]*project[[:space:]]*\(([A-Za-z0-9_.+-]+).*/\1/p' "${plugin_cmake}" | sed -n '1p')"
    fi

    if [[ -n "${detected_target}" ]]; then
        printf '%s' "${detected_target}"
        return
    fi

    if [[ "${normalized_request}" == *"-plugin" ]]; then
        printf '%s-vst3' "${normalized_request%-plugin}"
    else
        printf '%s-vst3' "${normalized_request}"
    fi
}

plugin_target=""
if [[ -n "${requested_plugin}" ]]; then
    plugin_target="$(resolve_plugin_target "${requested_plugin}")"
fi

plugin_source_dir=""
if [[ -n "${requested_plugin}" ]]; then
    normalized_plugin_dir="${requested_plugin#src/}"
    potential_plugin_source_dir="${script_dir}/src/${normalized_plugin_dir}"
    if [[ -f "${potential_plugin_source_dir}/CMakeLists.txt" ]]; then
        plugin_source_dir="${potential_plugin_source_dir}"
    fi
fi

if [[ -n "${plugin_source_dir}" ]]; then
    standalone_build_root="${script_dir}/build"
    standalone_build_dbg="${standalone_build_root}/${normalized_plugin_dir}-dbg"
    standalone_build_xcode="${standalone_build_root}/${normalized_plugin_dir}-xcode"

    rm -rf "${standalone_build_dbg}" "${standalone_build_xcode}"

    if [[ "$(uname -s)" == "Darwin" ]]; then
        cmake -S "${plugin_source_dir}" -B "${standalone_build_xcode}" -G Xcode
        cmake --build "${standalone_build_xcode}" --config Debug --target "${plugin_target}"
        cmake --build "${standalone_build_xcode}" --config Release --target "${plugin_target}"
    else
        cmake -S "${plugin_source_dir}" -B "${standalone_build_dbg}" -G "${build_generator}" -DCMAKE_BUILD_TYPE=Debug
        cmake --build "${standalone_build_dbg}"

        # might need to reboot your IDE/Text ed
        # Zed didn't pick up the file until I restarted the program
        ln -sf "${standalone_build_dbg}/compile_commands.json" "${script_dir}/compile_commands.json"
    fi

    exit 0
fi

rm -rf "${script_dir}/build-sys-dbg" "${script_dir}/build-xcode"

cmake -S "${script_dir}" -B "${script_dir}/build-sys-dbg" -G "${build_generator}" -DCMAKE_BUILD_TYPE=Debug
cmake --build "${script_dir}/build-sys-dbg"

# might need to reboot your IDE/Text ed
# Zed didn't pick up the file until I restarted the program
ln -sf "${script_dir}/build-sys-dbg/compile_commands.json" "${script_dir}/compile_commands.json"

if [[ "$(uname -s)" == "Darwin" ]]; then
    cmake -S "${script_dir}" -B "${script_dir}/build-xcode" -G Xcode

    if [[ -n "${plugin_target}" ]]; then
        cmake --build "${script_dir}/build-xcode" --config Release --target "${plugin_target}"
    else
        cmake --build "${script_dir}/build-xcode" --config Release
    fi
fi
