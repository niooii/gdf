#pragma once

#include <ser20/ser20.hpp>
#include <ser20/types/polymorphic.hpp>
#include <serde.h>
#include <vector>
#include <game/types.h>
#include <prelude.h>

struct ChunkLoadInfo {
    ivec3 cc;
    SERIALIZE_FIELDS(cc);
};

DECL_NET_EVENT(ChunkLoadEvent)
{
    std::vector<ChunkLoadInfo> loaded_chunks;
    SERIALIZE_EVENT_FIELDS(loaded_chunks)
};

DECL_NET_EVENT(TestTextEvent)
{
    std::string message;
    SERIALIZE_EVENT_FIELDS(message)
};

DECL_NET_EVENT(ChunkUpdateEvent)
{
    ivec3 chunk_coord;
    u8vec3 updated;
    SERIALIZE_EVENT_FIELDS(chunk_coord, updated)
};

DECL_NET_EVENT(PlayerMoveEvent)
{
    vec3 pos;
    SERIALIZE_EVENT_FIELDS(pos)
};
