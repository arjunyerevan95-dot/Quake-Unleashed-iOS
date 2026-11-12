# Asset-free renderer probe

This diagnostic is NOT UNLEASHED or a claim that its renderer works on iOS.

`probe.c` requires a current isolated GL4.3 core context, then really executes:
1. GLSL430 compute compilation/link.
2. SSBO GPU write and mapped readback of four deterministic values.
3. Image-store and framebuffer pixel readback.
4. GLSL430 vertex/fragment triangle draw and pixel readback.

All created GL objects are released. Requires an isolated context because bindings and state are modified.
`linux_egl.c` creates a surfaceless EGL core context; passes locally on Mesa llvmpipe GL4.5. This is a harness correctness test, not iOS or GPU performance evidence.

```
cc -std=c17 -Wall -Wextra -Werror probe.c linux_egl.c -ldl -o /tmp/probe
MESA_SHADER_CACHE_DISABLE=true /tmp/probe
cc -D_POSIX_C_SOURCE=200809L -DQVRU_OSMESA_CLI -std=c17 -Wall -Wextra -Werror probe.c osmesa_host.c -ldl -o /tmp/osmesa-probe
```

`osmesa_host.c` is independently authored and dynamically loads the named Mesa artifact. Strict mode removes version overrides and requests genuine GL4.3 core. MoltenVK lacks geometry shaders; the related BCD stack documents a maximum honest GL3.1 version, so strict failure is expected, not an excuse to claim success.

An explicitly labeled experimental mode raises the advertised version only to test whether the needed subset can execute. It never proves conformance, and reports the override prominently. This can be useful because current UNLEASHED shader stages appear to use vertex/fragment/compute rather than geometry/tessellation. Each additional renderer feature and actual engine shader must still be validated; this small test is necessary, not sufficient. Zink renderer is required, preventing silent softpipe fallback.

`App.m` is an **uncompiled** UIKit host. It allows one strict OR experimental test per app launch, saves `renderer-probe.txt`, and displays a CPU-read-back green pixel on success. It does not implement a live Metal presentation layer. This distinction matters for performance.

Before Apple compilation/package:
- Obtain verified iOS arm64 `libOSMesa.dylib` and `libMoltenVK.dylib` plus their complete dependency closure and notices. Current BCD run has not delivered these artifacts yet. Do not copy Wine/BCD GPL3 adapter code into this probe.
- Review exact dependency licences/provenance, load paths and signing. No engine or retail assets should be in this app.
- Compile `App.m`, `probe.c`, `osmesa_host.c` for iPhoneOS with UIKit/Foundation, ARC only for Objective-C, and package verified dylibs under Frameworks.
- Physical device test needed; no Apple runner spent yet.

## Minimal Apple build workflow

`.github/workflows/ios-renderer-probe.yml` is manual-only, with a 15-minute ceiling.
It fetches a public, whole-file hash-pinned donor IPA and extracts **only** the
seven allowlisted native-library/license members. `native-inputs.json` records
provenance; `extract_inputs.py` verifies whole archive and each member before
creating any output. No BCD app, Wine code or game data is packaged. The native
libraries are a cached dependency source, not a runtime dependency on BCD.
The output is a diagnostic IPA, not a playable UNLEASHED port. Native Apple
compilation and device execution remain unproven until the workflow/device test.
