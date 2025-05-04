#pragma once

#include <game/physics/aabb.h>
#include <gdfe/prelude.h>

namespace Components {
    struct Health {
        f32 val;
    };

    struct Rotation {
        f32 pitch = 0;
        f32 yaw   = 0;
    };
} // namespace Components
