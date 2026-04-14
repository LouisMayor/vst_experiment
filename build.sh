#!/bin/bash
set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"

rm -rf "${script_dir}/build-sys-dbg" "${script_dir}/build-xcode"

build_generator="Unix Makefiles"
if command -v ninja >/dev/null 2>&1; then
    build_generator="Ninja"
fi

cmake -S "${script_dir}" -B "${script_dir}/build-sys-dbg" -G "${build_generator}" -DCMAKE_BUILD_TYPE=Debug
cmake --build "${script_dir}/build-sys-dbg"

# might need to reboot your IDE/Text ed
# Zed didn't pick up the file until I restarted the program
ln -sf build-sys-dbg/compile_commands.json compile_commands.json

cmake -S "${script_dir}" -B "${script_dir}/build-xcode" -G Xcode
xcodebuild -project "${script_dir}/build-xcode/vst.xcodeproj" -scheme gain-vst3 -configuration Release build
xcodebuild -project "${script_dir}/build-xcode/vst.xcodeproj" -scheme sin_tone-vst3 -configuration Release build
