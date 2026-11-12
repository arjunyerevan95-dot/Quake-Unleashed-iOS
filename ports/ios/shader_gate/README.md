# Actual engine shader-stage gate

Device gate 1 (user screenshots, 2026-11-12): A18 Pro / Zink Vulkan 1.4 /
MoltenVK rejected strict GL4.3 core; explicit experimental version override
passed compute, SSBO, image-store and triangle readback. This does not establish
GL4.3 conformance or game compatibility.

This next asset-free gate exports 38 actual upstream shader stages from pinned
UNLEASHED next commit 9192b03797cfa3a724d1f71c4b42609ff52865a4. It compiles
42 stage variants, including alias-model vertex/fragment types 0, 1, 2 (skeletal
skinning included), using BINDLESS=0, REVERSED_Z=0 and baseline effect defines.
Linux llvmpipe: 42/42 pass. iOS result remains pending.

This is stage compilation only: no program linking, full effect permutations,
resource-limit workload, bone-pose draw, presentation or performance claims.
The next required gates are actual program linking and a textured/skinned moving
scene with a real presentation path, before importing licensed game data.

The generated GLSL retains upstream GPL-2.0-or-later terms. Its exact editable
source is the pinned upstream gl_shaders.h and included vr headers. The workflow
fetches that source explicitly, retains the upstream license in the corpus, and
records source commit and per-stage hashes. No retail assets are needed.
