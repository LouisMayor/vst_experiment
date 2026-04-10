#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source_dir="${script_dir}/src/thirdparty/vst3projectgenerator"
build_root="${script_dir}/build"
build_dir="${build_root}/vst3projectgenerator"
build_type="${BUILD_TYPE:-Release}"
target="VST3_Project_Generator"

mkdir -p "${build_root}"

cmake -S "${source_dir}" -B "${build_dir}" -DCMAKE_BUILD_TYPE="${build_type}"
cmake --build "${build_dir}" --config "${build_type}" --target "${target}"

artifact_dir="${build_dir}/${build_type}"
artifact="${artifact_dir}/${target}"

if [[ "${OSTYPE:-}" == darwin* ]] && [[ -d "${artifact}.app" ]]; then
  rm -rf "${build_root}/${target}.app"
  cp -R "${artifact}.app" "${build_root}/"
elif [[ -d "${artifact}" ]]; then
  rm -rf "${build_root}/${target}"
  cp -R "${artifact}" "${build_root}/"
elif [[ -f "${artifact}" ]]; then
  cp "${artifact}" "${build_root}/"
elif [[ -f "${artifact}.exe" ]]; then
  cp "${artifact}.exe" "${build_root}/"
else
  echo "Built target, but could not find artifact at: ${artifact}" >&2
  exit 1
fi
