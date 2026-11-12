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

## Gate 3
User device result for v2: 42 stage tests, zero failures on A18 Pro experimental
Zink context. V3 now links 24 baseline programs (including three alias-model
pose formats and six compute programs), then uses the actual upstream GUI
program to draw generated geometry with a generated green texture. It validates
the expected RGB pixel readback and presents that pixel through UIKit.
Linux baseline: 42 stages and 24 links pass; textured GUI readback passes.
iPhone v3 pending. This is not a moving scene, skinned-model draw, native Metal
swapchain or performance test. World-depth and effect permutations beyond the
baseline are not covered.
