#!/bin/bash
set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"
requested_plugin=""
requested_config="Release"

if [[ $# -ge 1 ]]; then
    arg1_lower="$(echo "$1" | tr '[:upper:]' '[:lower:]')"
    if [[ "$arg1_lower" == "debug" || "$arg1_lower" == "release" ]]; then
        requested_config="$1"
        if [[ $# -ge 2 ]]; then
            requested_plugin="$2"
        fi
    else
        requested_plugin="$1"
        if [[ $# -ge 2 ]]; then
            arg2_lower="$(echo "$2" | tr '[:upper:]' '[:lower:]')"
            if [[ "$arg2_lower" == "debug" || "$arg2_lower" == "release" ]]; then
                requested_config="$2"
            else
                printf 'Invalid configuration: %s. Use "debug" or "release".\n' "$2" >&2
                exit 1
            fi
        fi
    fi
fi

build_generator="Unix Makefiles"
compile_commands_inputs=()

if command -v ninja >/dev/null 2>&1; then
    build_generator="Ninja"
fi

resolve_plugin_dir() {
    local request="$1"
    local normalized_request="${request#src/}"
    local plugin_source_dir="${script_dir}/src/${normalized_request}"

    if [[ -f "${plugin_source_dir}/CMakeLists.txt" ]]; then
        printf '%s' "${normalized_request}"
        return
    fi

    if [[ "${normalized_request}" == *"-vst3" ]]; then
        local mapped_plugin_dir="${normalized_request%-vst3}-plugin"
        if [[ -f "${script_dir}/src/${mapped_plugin_dir}/CMakeLists.txt" ]]; then
            printf '%s' "${mapped_plugin_dir}"
            return
        fi
    fi

    printf ''
}

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

build_plugin() {
    local plugin_dir="$1"
    local plugin_target="$2"
    local build_config="$3"
    local config_lower="$(echo "$build_config" | tr '[:upper:]' '[:lower:]')"
    local plugin_source_dir="${script_dir}/src/${plugin_dir}"
    local standalone_build_root="${script_dir}/build"
    local standalone_build_dir="${standalone_build_root}/${plugin_dir}-${config_lower}"
    local standalone_build_xcode="${standalone_build_root}/${plugin_dir}-xcode"
    local cmake_config_args=(-DCMAKE_BUILD_TYPE="${build_config}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)

    printf 'Building %s (target: %s, config: %s)\n' "${plugin_dir}" "${plugin_target}" "${build_config}"

    rm -rf "${standalone_build_dir}"
    if [[ "$(uname -s)" == "Darwin" ]]; then
        rm -rf "${standalone_build_xcode}"
    fi

    if [[ "$(uname -s)" == "Darwin" ]]; then
        cmake_config_args+=("-DCMAKE_OBJCXX_COMPILE_OBJECT=<CMAKE_OBJCXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
    fi

    cmake -S "${plugin_source_dir}" -B "${standalone_build_dir}" -G "${build_generator}" "${cmake_config_args[@]}"

    if [[ "$(uname -s)" == "Darwin" ]]; then
        cmake -S "${plugin_source_dir}" -B "${standalone_build_xcode}" -G Xcode
        cmake --build "${standalone_build_xcode}" --config "${build_config}" --target "${plugin_target}"
    else
        cmake --build "${standalone_build_dir}"
    fi

    compile_commands_inputs+=("${standalone_build_dir}/compile_commands.json")
}

write_compile_commands_file() {
    local output_file="${script_dir}/compile_commands.json"

    if [[ ${#compile_commands_inputs[@]} -eq 0 ]]; then
        return
    fi

    rm -f "${output_file}"

    if [[ ${#compile_commands_inputs[@]} -eq 1 ]]; then
        ln -sf "${compile_commands_inputs[0]}" "${output_file}"
        return
    fi

    python3 - "${output_file}" "${compile_commands_inputs[@]}" <<'PY'
import json
import os
import sys

output_path = sys.argv[1]
input_paths = sys.argv[2:]

merged_entries = []
seen = set()

for path in input_paths:
    if not os.path.isfile(path):
        continue

    with open(path, "r", encoding="utf-8") as handle:
        data = json.load(handle)

    if not isinstance(data, list):
        continue

    for entry in data:
        if not isinstance(entry, dict):
            continue

        key = (entry.get("directory"), entry.get("file"))
        if key in seen:
            continue

        seen.add(key)
        merged_entries.append(entry)

with open(output_path, "w", encoding="utf-8") as handle:
    json.dump(merged_entries, handle, indent=2)
    handle.write("\n")
PY
}

if [[ -n "${requested_plugin}" ]]; then
    plugin_source_dir="$(resolve_plugin_dir "${requested_plugin}")"
    if [[ -z "${plugin_source_dir}" ]]; then
        printf 'Usage: %s [plugin-dir-or-target] [debug|release]\n' "$(basename "$0")" >&2
        printf 'Example: %s gain-plugin\n' "$(basename "$0")" >&2
        printf 'Example: %s gain-plugin debug\n' "$(basename "$0")" >&2
        printf 'If no plugin is provided, all plugins in src/*-plugin are built.\n' >&2
        exit 1
    fi

    plugin_target="$(resolve_plugin_target "${requested_plugin}")"
    build_plugin "${plugin_source_dir}" "${plugin_target}" "${requested_config}"
    write_compile_commands_file
    exit 0
fi

shopt -s nullglob
plugin_cmake_files=("${script_dir}"/src/*-plugin/CMakeLists.txt)
shopt -u nullglob

if [[ ${#plugin_cmake_files[@]} -eq 0 ]]; then
    printf 'No plugin directories found in %s/src/*-plugin\n' "${script_dir}" >&2
    exit 1
fi

for plugin_cmake_file in "${plugin_cmake_files[@]}"; do
    plugin_dir="$(basename "$(dirname "${plugin_cmake_file}")")"
    plugin_target="$(resolve_plugin_target "${plugin_dir}")"
    build_plugin "${plugin_dir}" "${plugin_target}" "${requested_config}"
done

write_compile_commands_file
