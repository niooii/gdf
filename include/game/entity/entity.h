#pragma once

#include <gdfe/core.h>
#include <game/physics/aabb.h>

namespace Components {
    struct Health {
        f32 val;
    };
}

typedef struct Entity {
    f32 health;
    GDF_BOOL damagable;
    void* parent;

    vec3 vel;
    vec3 accel;

    AxisAlignedBoundingBox aabb;
    GDF_BOOL grounded;
} Entity;