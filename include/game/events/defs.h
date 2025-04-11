#pragma once

#include <game/events.h>
#include <ser20/ser20.hpp>
#include <ser20/types/polymorphic.hpp>
#include <serde.h>
#include <vector>
#include <game/types.h>
#include <game/world.h>

DECL_SERDE_EVENT(ChunkLoadEvent)
{
    std::vector<ivec3> loaded_chunks;
    SERIALIZE_FIELDS(loaded_chunks)
};

DECL_SERDE_EVENT(TestTextEvent)
{
    std::string message;
    SERIALIZE_FIELDS(message)
};

DECL_SERDE_EVENT(ChunkUpdateEvent)
{
    ivec3 chunk_coord;
    u8vec3 updated;
    SERIALIZE_FIELDS(chunk_coord, updated)
};

DECL_SERDE_EVENT(PlayerMoveEvent)
{
    vec3 pos;
    SERIALIZE_FIELDS(pos)
};
