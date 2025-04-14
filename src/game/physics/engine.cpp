#include <game/physics/engine.h>
#include <game/world.h>

PhysicsSimulation::PhysicsSimulation(const SimulationCreateInfo& info, const World* world)
    : world{world}
{
    gravity_ = info.gravity;
    gravity_active_ = info.gravity_active;
    air_drag_ = info.air_drag;
    ground_drag_ = info.ground_drag;
    terminal_velocity_ = info.terminal_velocity;
    registry_ = info.entity_registry_p;
}

// TODO! THIS IS SO HORRIBLE
void PhysicsSimulation::update(f32 dt)
{
    // TODO! stupid hack for now to test with absurd chunk loading times and not
    // fall thru the ground
    if (dt > 0.5)
        dt = 0.5;

    // TODO! optimize, look into SIMD
    vec3 effective_gravity = gravity_active_ ? gravity_ : vec3_zero();
    vec3 net_accel;

    auto view = registry_->view<Components::Velocity, Components::AabbCollider>();

    for(auto [entity, vel_comp, collider]: view.each())
    {
        vec3& vel = vel_comp.vec;
        f32 drag = collider.is_grounded ? ground_drag_ : air_drag_;
        // TODO! this aint quite right..
        if (gravity_active_)
        {
            f32 drag_factor = 1.0f - (drag * dt);
            drag_factor = MAX(0, drag_factor);
            vel.x *= drag_factor;
            vel.z *= drag_factor;
        }

        // TODO! acceleration component
        vec3 accel = vec3_zero();
        net_accel = vec3_add(accel, effective_gravity);

        vec3 deltas = vec3_new(
            vel.x * dt + 0.5f * net_accel.x * dt * dt,
            vel.y * dt + 0.5f * net_accel.y * dt * dt,
            vel.z * dt + 0.5f * net_accel.z * dt * dt
        );

        vel.x = vel.x + net_accel.x * dt;
        // cap velocity gain at a terminal velocity
        // TODO! this should be for all axis - a deceleration force, not yk, flat cap
        if (vel.y > terminal_velocity_)
        {
            f32 t_vy = vel.y + net_accel.y * dt;
            if (t_vy < terminal_velocity_)
                vel.y = terminal_velocity_;
            else
                vel.y = t_vy;
        }
        vel.z = vel.z + net_accel.z * dt;

        // TODO! eliminate phasing through blocks at high velocities
        // with raycasting

        AxisAlignedBoundingBox t_aabb = collider.aabb;
        aabb_translate(&t_aabb, deltas);

        BlockTouchingResult results[64];
        u32 results_len = world->get_blocks_touching(
            &t_aabb,
            results,
            std::size(results)
        );

        // Check for floor (only if translated aabb's y is below the current one)
        GDF_BOOL ground_found = GDF_FALSE;
        GDF_BOOL should_check_ground =
            t_aabb.min.y <= collider.aabb.min.y;
        f32 ground_y = 0;
        if (should_check_ground)
        {
            vec3 corner;
            for (u32 i = 0; i < 4; i++)
            {
                switch (i)
                {
                    case 0:
                    corner = aabb_bot_left(&t_aabb);
                    break;
                    case 1:
                    corner = aabb_bot_left_back(&t_aabb);
                    break;
                    case 2:
                    corner = aabb_bot_right(&t_aabb);
                    break;
                    case 3:
                    corner = aabb_bot_right_back(&t_aabb);
                    break;
                }
                // REPLACE WITH RAYCAST (or just handle slabs well)
                ivec3 cc = world_pos_to_chunk_coord(corner);
                Block* block = world->get_block(corner);
                if (block != NULL)
                {
                    ground_found = GDF_TRUE;
                    // without +1 is the coordinate from the bottom left
                    ground_y = cc.y * CHUNK_SIZE + block->y_rel + 1;
                    // set aabb at appropriate y level
                    f32 y_offset = ground_y - t_aabb.min.y;
                    vec3 offset_vec = vec3_new(0, y_offset, 0);
                    aabb_translate(&t_aabb, offset_vec);
                    vec3_add_to(&deltas, offset_vec);
                    vel.y = 0;
                    break;
                }
            }
        }

        EntityBlockCollisionEvent e;
        e.entity = entity;
        e.touched.reserve(8);
        for (u32 i = 0; i < results_len; i++)
        {
            BlockTouchingResult* r = results + i;
            if (aabb_intersects(&t_aabb, &r->box))
            {
                vec3 resolution = aabb_get_intersection_resolution(&t_aabb, &r->box);
                // discard all blocks on ground level, but still fire event
                // (avoids some weird issues with edge collisions)
                if (ground_found && r->box.max.y == ground_y)
                {
                    // GDF_EventFire(GDF_EVENT_BLOCK_TOUCHED, r->block, ctx);
                    continue;
                }

                vec3_add_to(&deltas, resolution);
                aabb_translate(&t_aabb, resolution);
                // zero velocity
                if (resolution.x != 0)
                {
                    vel.x = 0;
                }
                if (resolution.y != 0)
                {
                    vel.y = 0;
                }
                if (resolution.z != 0)
                {
                    vel.z = 0;
                }
                e.touched.push_back(r->block);
            }
        }
        // update grounded status
        collider.is_grounded = ground_found;
        // this fluctuates for negative y apparently lol
        LOG_DEBUG("GROUND FOUND: %d", ground_found);
        Services::Events::queue_dispatch(e);

        aabb_translate(&collider.aabb, deltas);
    }
}
