#!/bin/bash
set -e

cd "$(dirname "$0")"

rm -rf build
mkdir -p build
cd build

cmake .. -G Xcode
xcodebuild -project vst.xcodeproj -scheme gain-vst3 -configuration Release build
