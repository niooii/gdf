#include <gdfe/def.h>

#ifndef DEFINE_EVENT
#define DEFINE_EVENT(name, fields)
#endif

DEFINE_EVENT(
    ChunkLoadEvent,
    ivec3 chunk_coord;
)
DEFINE_EVENT(
    ChunkUpdateEvent,
    ivec3 chunk_coord;
)
DEFINE_EVENT(
    PlayerMoveEvent,
    vec3 pos;
)
DEFINE_EVENT(
    TestTextEvent,
    const char* message = nullptr;
)

#undef DEFINE_EVENT