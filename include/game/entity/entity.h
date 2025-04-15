#pragma once

#include <gdfe/core.h>
#include <game/physics/aabb.h>

namespace Components {
    struct Health {
        f32 val;
    };

    struct Rotation {
        f32 pitch;
        f32 yaw;
    };
}
