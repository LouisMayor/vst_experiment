#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
generator_script="${script_dir}/src/thirdparty/vst3projectgenerator/script/GenerateVST3Plugin.cmake"
output_root="${script_dir}/src"

require_command() {
    local cmd="$1"
    if ! command -v "${cmd}" >/dev/null 2>&1; then
        echo "Error: '${cmd}' is required but was not found in PATH." >&2
        exit 1
    fi
}

trim() {
    local value="$1"
    value="${value#"${value%%[![:space:]]*}"}"
    value="${value%"${value##*[![:space:]]}"}"
    printf '%s' "${value}"
}

to_slug() {
    local value="$1"
    printf '%s' "${value}" \
        | tr '[:upper:]' '[:lower:]' \
        | sed -E 's/[^a-z0-9]+/-/g; s/^-+//; s/-+$//; s/-+/-/g'
}

to_pascal() {
    local value="$1"
    printf '%s' "${value}" \
        | sed -E 's/[^a-zA-Z0-9]+/ /g' \
        | awk '{
            for (i = 1; i <= NF; ++i) {
                token = $i
                first = substr(token, 1, 1)
                rest = substr(token, 2)
                printf toupper(first) tolower(rest)
            }
        }'
}

require_command cmake

if [[ ! -f "${generator_script}" ]]; then
    echo "Error: Generator script was not found at: ${generator_script}" >&2
    exit 1
fi

echo "VST Project Setup"
echo "-----------------"

vst_name=""
while [[ -z "${vst_name}" ]]; do
    read -r -p "VST name: " vst_name_input
    vst_name="$(trim "${vst_name_input}")"
    if [[ -z "${vst_name}" ]]; then
        echo "Please enter a non-empty VST name."
    fi
done

plugin_category=""
while [[ -z "${plugin_category}" ]]; do
    echo "Select VST type:"
    echo "  1) Instrument"
    echo "  2) Fx"
    read -r -p "Choice [1-2]: " plugin_type_choice

    case "$(trim "${plugin_type_choice}")" in
        1|Instrument|instrument)
            plugin_category="Instrument"
            ;;
        2|Fx|fx|FX)
            plugin_category="Fx"
            ;;
        *)
            echo "Invalid choice. Enter 1 for Instrument or 2 for Fx."
            ;;
    esac
done

name_slug="$(to_slug "${vst_name}")"
if [[ -z "${name_slug}" ]]; then
    echo "Error: VST name must include at least one alphanumeric character." >&2
    exit 1
fi

plugin_dir_name="${name_slug}-plugin"
cmake_project_name="${name_slug}-vst3"
plugin_class_name="$(to_pascal "${vst_name}")"

if [[ -z "${plugin_class_name}" ]]; then
    plugin_class_name="GeneratedPlugin"
fi
if [[ "${plugin_class_name}" =~ ^[0-9] ]]; then
    plugin_class_name="Plugin${plugin_class_name}"
fi

vendor_namespace="${plugin_class_name}Vendor"
identifier_slug="${name_slug//-/_}"
plugin_identifier="com.example.vst3.${identifier_slug}"
plugin_output_dir="${output_root}/${plugin_dir_name}"

if [[ -e "${plugin_output_dir}" ]]; then
    echo "Error: Target directory already exists: ${plugin_output_dir}" >&2
    exit 1
fi

echo ""
echo "Generating project..."

cmake \
    -DSMTG_PLUGIN_NAME_CLI="${plugin_dir_name}" \
    -DSMTG_PLUGIN_CATEGORY_CLI="${plugin_category}" \
    -DSMTG_CMAKE_PROJECT_NAME_CLI="${cmake_project_name}" \
    -DSMTG_PLUGIN_BUNDLE_NAME_CLI="${cmake_project_name}" \
    -DSMTG_PLUGIN_IDENTIFIER_CLI="${plugin_identifier}" \
    -DSMTG_VENDOR_NAMESPACE_CLI="${vendor_namespace}" \
    -DSMTG_PLUGIN_CLASS_NAME_CLI="${plugin_class_name}" \
    -DSMTG_GENERATOR_OUTPUT_DIRECTORY_CLI="${output_root}" \
    -P "${generator_script}"

if [[ ! -d "${plugin_output_dir}" ]]; then
    echo "Error: Generator completed but output folder is missing: ${plugin_output_dir}" >&2
    exit 1
fi

echo ""
echo "Done."
echo "- Project directory: ${plugin_output_dir}"
echo "- VST category in cids: ${plugin_category}"
echo ""
echo "Next step: add 'add_subdirectory(src/${plugin_dir_name})' to your top-level CMakeLists.txt."
