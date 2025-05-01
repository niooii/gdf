#pragma once

#include <gdfe/prelude.h>
#include <game/physics/aabb.h>

namespace Components {
    struct Health {
        f32 val;
    };

    struct Rotation {
        f32 pitch = 0;
        f32 yaw = 0;
    };
}
