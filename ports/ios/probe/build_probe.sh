#!/bin/bash
# Independent diagnostic packaging, not a game build. Requires preverified native dependencies.
set -euo pipefail
if [ "$#" -ne 2 ]; then echo "usage: $0 VERIFIED_DEPENDENCY_DIR OUTPUT_DIR" >&2; exit 2; fi
here=$(cd "$(dirname "$0")" && pwd)
deps=$(cd "$1" && pwd)
mkdir -p "$2"; out=$(cd "$2" && pwd)
[ -f "$deps/SOURCE_NOTICES.txt" ] || { echo 'Missing dependency provenance/notices' >&2; exit 2; }
[ -f "$deps/SHA256SUMS" ] || { echo 'Missing dependency checksum manifest' >&2; exit 2; }
for name in libOSMesa.dylib libMoltenVK.dylib; do
 [ -f "$deps/$name" ] || { echo "Missing $name" >&2; exit 2; }
 grep -F "$name" "$deps/SHA256SUMS" >/dev/null || { echo "Unhashed $name" >&2; exit 2; }
done
(cd "$deps" && shasum -a 256 -c SHA256SUMS)
# Verify every bundled dylib is device arm64 and dependencies cannot escape the bundle.
python3 - "$deps" <<'PY'
import pathlib,subprocess,sys
p=pathlib.Path(sys.argv[1]);libs=list(p.glob('*.dylib'));names={x.name for x in libs}
for lib in libs:
 archs=subprocess.check_output(['xcrun','lipo','-archs',str(lib)],text=True).split()
 if 'arm64' not in archs: raise SystemExit(f'{lib.name}: no arm64 slice')
 load=subprocess.check_output(['xcrun','otool','-l',str(lib)],text=True)
 if 'platform IOS' not in load and 'platform 2' not in load:
  raise SystemExit(f'{lib.name}: iPhoneOS LC_BUILD_VERSION not verified')
 deps=subprocess.check_output(['xcrun','otool','-L',str(lib)],text=True).splitlines()[1:]
 for line in deps:
  dep=line.strip().split(' (',1)[0]
  if dep.startswith(('/usr/lib/','/System/Library/')): continue
  if dep==f'@rpath/{lib.name}': continue
  if dep.startswith('@rpath/') and dep[7:] in names: continue
  raise SystemExit(f'{lib.name}: unsupported/unbundled dependency {dep}')
PY
app="$out/Payload/UnleashedRendererProbe.app"
[ ! -e "$app" ] || { echo 'Use a fresh output directory; refusing to overwrite a prior app' >&2; exit 2; }
mkdir -p "$app/Frameworks"
cp "$deps"/*.dylib "$app/Frameworks/"
cp "$deps/SOURCE_NOTICES.txt" "$deps/SHA256SUMS" "$deps/provenance.json" "$app/"
mkdir -p "$app/licenses"
cp "$deps/"*.txt "$deps/"*.rst "$app/licenses/"
sdk=$(xcrun --sdk iphoneos --show-sdk-path)
probe_source="$here/probe.c"
extra_flags=()
extra_objects=()
if [ "${QVR_SHADER_GATE:-0}" = 1 ]; then
 probe_source="$here/../shader_gate/compile_stages.c"
 extra_flags=(-DQVR_SHADER_GATE=1)
 [ -d "$QVR_SHADER_CORPUS" ] || { echo "Missing shader corpus" >&2; exit 2; }
 cp -R "$QVR_SHADER_CORPUS" "$app/shader-corpus"
fi
if [ "${QVR_SHADER_GATE:-0}" = 1 ]; then
 xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" -miphoneos-version-min=17.0 -std=c17 -O2 -Wall -Wextra -Werror -c "$here/../shader_gate/draw_gui.c" -o "$out/draw_gui.o"
 extra_objects=("$out/draw_gui.o")
fi
for src in probe osmesa_host; do
 input="$here/$src.c"; [ "$src" != probe ] || input="$probe_source"
 xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" -miphoneos-version-min=17.0 -std=c17 -O2 -Wall -Wextra -Werror -c "$input" -o "$out/$src.o"
done
xcrun --sdk iphoneos clang -arch arm64 -isysroot "$sdk" -miphoneos-version-min=17.0 -fobjc-arc -O2 "${extra_flags[@]}" "$here/App.m" "$out/probe.o" "$out/osmesa_host.o" "${extra_objects[@]}" -framework UIKit -framework Foundation -framework CoreGraphics -Wl,-rpath,@executable_path/Frameworks -o "$app/UnleashedRendererProbe"
python3 - "$app/Info.plist" <<'PY'
import plistlib,sys
with open(sys.argv[1],'wb') as f: plistlib.dump(dict(CFBundleIdentifier='am.arjunkl.unleashed.rendererprobe',CFBundleExecutable='UnleashedRendererProbe',CFBundleName='UNLEASHED Renderer Probe',CFBundlePackageType='APPL',CFBundleShortVersionString='0.0.1',CFBundleVersion='3',MinimumOSVersion='17.0',UIDeviceFamily=[1,2],LSRequiresIPhoneOS=True,UILaunchScreen={},UIFileSharingEnabled=True,LSSupportsOpeningDocumentsInPlace=True,UISupportedInterfaceOrientations=['UIInterfaceOrientationLandscapeLeft','UIInterfaceOrientationLandscapeRight']),f)
PY
for lib in "$app/Frameworks/"*.dylib; do codesign --force --sign - "$lib"; done
codesign --force --sign - "$app"
(cd "$out" && zip -qr UnleashedRendererProbe-unsigned.ipa Payload)
shasum -a 256 "$out/UnleashedRendererProbe-unsigned.ipa"
echo 'Diagnostic only. Requires user sideload signing. No game data or gameplay included.'
