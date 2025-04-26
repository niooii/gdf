#pragma once

#include <ser20/ser20.hpp>
#include <ser20/types/polymorphic.hpp>
#include <serde.h>
#include <vector>
#include <game/types.h>
#include <prelude.h>

DECL_NET_EVENT(PlayerMoveEvent)
{
    vec3 pos;
    SERIALIZE_EVENT_FIELDS(pos)
};
