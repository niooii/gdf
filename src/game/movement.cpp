#include <game/movement.h>
#include <game/entity/humanoid.h>

void jump(HumanoidEntity* humanoid, vec3 up_vec, f32 jump_power)
{
    Entity* entity = &humanoid->base;
    vec3_add_to(&entity->vel, vec3_mul_scalar(up_vec, 7.0*jump_power));
}

// Set to 1 for max
const f32 AIR_CONTROL = 0.5;
// How significantly a player is moving in a direction 
const f32 AIR_CONTROL_THRESHOLD = 0.1f;

void player_apply_movement(
    HumanoidEntity* humanoid, 
    i8 x_input,
    i8 z_input,
    vec3* forward,
    vec3* right,
    f32 dt,
    GDF_BOOL just_jumped,
    f32 speed
)
{
    Entity* entity = &humanoid->base;

    vec3 dv = {0};

    vec3 right_vec = vec3_new(right->x, 0, right->z);
    vec3_normalize(&right_vec);
    vec3 forward_vec = vec3_new(forward->x, 0, forward->z);
    vec3_normalize(&forward_vec);

    f32 real_speed = speed * 100;
    if (just_jumped)
        real_speed *= 4;
    else if (!entity->grounded)
        real_speed *= 0.3;

    vec3_add_to(&dv, vec3_mul_scalar(forward_vec, z_input));
    vec3_add_to(&dv, vec3_mul_scalar(right_vec, x_input));
    
    if (x_input != 0 || z_input != 0) 
    {
        vec3_normalize(&dv);
    }

    dv = vec3_mul_scalar(dv, real_speed * dt);

    // vec3 horizontal_vel = vec3_new(physics->vel.x, 0, physics->vel.z);
    // f32 current_speed = vec3_length(horizontal_vel);

    // if (!physics->grounded) {
    //     vec3 vel_dir = horizontal_vel;
    //     if (current_speed > AIR_CONTROL_THRESHOLD) {
    //         vec3_normalize(&vel_dir);
            
    //         f32 movement_alignment = vec3_dot(vel_dir, dv);

    //         // LOG_DEBUG("DOT PRODUCT: %f", movement_alignment);
            
    //         // Allows for change in velocity very close to 180 degs
    //         if (movement_alignment < 0.1f) {
    //             return;
    //         }
    //     }
        
    //     // Apply air control only when changing direction or moving slowly
    //     if (!just_jumped)
    //         dv = vec3_mul_scalar(dv, AIR_CONTROL);
    // }

    vec3_add_to(&entity->vel, dv);
}

void dash(
    HumanoidEntity* humanoid, 
    f32 dash_power,
    vec3 forward
)
{
    if (!humanoid->movement_state.can_dash)
        return;
    vec3 dash_vec = vec3_mul_scalar(forward, dash_power * 40);
    dash_vec.y *= 0.4;
    // vec3_add_to(&humanoid->base.vel, dash_vec);
    humanoid->base.vel = dash_vec;
    humanoid->movement_state.in_dash = GDF_TRUE;
    humanoid->movement_state.can_dash = GDF_FALSE;
}

void movement_update(HumanoidEntity* hum)
{
    if (hum->movement_state.in_dash)
    {
        if (hum->base.grounded || vec3_length(hum->base.vel) < 10)
        {
            hum->movement_state.in_dash = GDF_FALSE;
            LOG_DEBUG("DASH FINISH");
        }
    }

    if (hum->base.grounded)
        hum->movement_state.can_dash = GDF_TRUE;
}