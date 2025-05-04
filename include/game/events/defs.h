#pragma once

#include <game/types.h>
#include <prelude.h>
#include <ser20/ser20.hpp>
#include <ser20/types/polymorphic.hpp>
#include <serde.h>
#include <vector>

DECL_PACKET(PlayerMoveEvent)
{
    vec3 pos;
    SERIALIZE_PACKET_FIELDS(pos)
};
