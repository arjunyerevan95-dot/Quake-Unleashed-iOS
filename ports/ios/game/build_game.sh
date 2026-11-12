#!/bin/bash
set -euo pipefail
src="$1"; sdl="$2"; inputs="$3"; out="$4"
mkdir -p "$out"
cmake -S "$src" -B "$out/build" -G Xcode \
 -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos \
 -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0 \
 -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO \
 -DQVR_SDL_SOURCE="$sdl" -DQVR_NATIVE_INPUTS="$inputs" \
 -DQVR_OPENXR=OFF -DQVR_PCH=OFF -DQVR_MIMALLOC=OFF -DENABLE_USERDIRS=OFF
cmake --build "$out/build" --config Release --parallel 3
app="$out/Payload/Unleashed.app"
mkdir -p "$out/Payload"
cp -R "$out/build/Release-iphoneos/ironwail.app" "$app"
mkdir -p "$app/Frameworks" "$app/Notices"
cp "$inputs"/*.dylib "$app/Frameworks/"
cp "$inputs/SOURCE_NOTICES.txt" "$app/Notices/"
cp "$src/LICENSE.txt" "$app/Notices/UNLEASHED-LICENSE.txt"
cp "$sdl/LICENSE.txt" "$app/Notices/SDL-LICENSE.txt"
for lib in "$app/Frameworks/"*.dylib; do codesign --force --sign - "$lib"; done
codesign --force --sign - "$app"
(cd "$out" && zip -qry Unleashed-launch-candidate.ipa Payload)
shasum -a 256 "$out/Unleashed-launch-candidate.ipa"
