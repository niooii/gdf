#pragma once
#include <game/prelude.h>

#include "aabb.h"

namespace Components
{
    struct Velocity {
        vec3 vec;
    };

    struct AabbCollider {
        AxisAlignedBoundingBox aabb;
        bool is_grounded;
    };
}

class World;

struct SimulationCreateInfo {
    f32 ground_drag;
    f32 air_drag;
    // TODO! this is only for negative Y, so
    // only negative numbers will work
    f32 terminal_velocity;
    vec3 gravity;
    GDF_BOOL gravity_active;
    // The lifetime of this registry must outlive the simulation
    ecs::Registry* entity_registry_p;
};

// The lifetime of a physics simulation must outlive it's registry.
class PhysicsSimulation {
    ecs::Registry* registry_;

    // TODO! calculate gravity on the fly heh
    // chibaku tensei....
    f32 terminal_velocity_;
    f32 air_drag_;
    f32 ground_drag_;
    vec3 gravity_;
    bool gravity_active_;

    // TODO! can find a way to decouple from world?
    const World* world;

public:
    PhysicsSimulation(const SimulationCreateInfo& info, const World* world);
    ~PhysicsSimulation();

    void update(f32 dt);
};