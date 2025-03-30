#include <gdfe/def.h>

#define FIELDS(...) __VA_ARGS__
#ifndef DEFINE_EVENT
#define DEFINE_EVENT(name, fields, ...)
#endif


/* WORLD EVENTS */
DEFINE_EVENT(
    ChunkLoadEvent,
    FIELDS(
        ivec3 chunk_coord;
    ),
    chunk_coord
)

DEFINE_EVENT(
    ChunkUpdateEvent,
    FIELDS(
        ivec3 chunk_coord;
    ),
    chunk_coord
)

DEFINE_EVENT(
    PlayerMoveEvent,
    FIELDS(
        vec3 pos;
    ),
    pos
)

/* Temporary test events */
DEFINE_EVENT(
    TestTextEvent,
    FIELDS(
        std::string message;
    ),
    message
)

#undef DEFINE_EVENT