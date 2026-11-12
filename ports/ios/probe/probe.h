/* New asset-free diagnostic code. No engine or game assets. */
#pragma once
#include <stddef.h>
typedef void *(*qvru_get_proc)(const char *name);
typedef struct { int passed; char detail[2048]; unsigned char rgba[4]; } qvru_probe_result;
/* Caller owns a current, isolated desktop core GL4.3 context. Destroys all objects it creates. */
qvru_probe_result qvru_probe(qvru_get_proc get);
/* Execute once in a dedicated probe process; Mesa env/context globals aren't reusable. */
qvru_probe_result qvru_osmesa_probe(const char *mesa, const char *vulkan, int experimental_override);
