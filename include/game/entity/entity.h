#pragma once

#include <gdfe/core.h>
#include <game/physics/aabb.h>

typedef enum ENTITY_TYPE {
    ENTITY_TYPE_BASE = 0,
    // "parent" is safe to be casted to HumanoidEntity*
    ENTITY_TYPE_HUMANOID
} ENTITY_TYPE;

namespace Components
{
    struct Health {
        f32 val;
    };
}

typedef struct Entity {
    f32 health;
    GDF_BOOL damagable;
    ENTITY_TYPE type;
    void* parent;

    vec3 vel;
    vec3 accel;

    AxisAlignedBoundingBox aabb;
    GDF_BOOL grounded;
} Entity;