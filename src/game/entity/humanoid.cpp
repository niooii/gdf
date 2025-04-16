#include <game/entity/humanoid.h>
#include <game/physics/engine.h>
#include <game/world.h>
#include <game/entity/entity.h>

using namespace Systems;
using namespace Components;

/* State function definitions */

HumanoidMovementController::HumanoidMovementController(ecs::Entity entity)
    : ctx{new MovementContext{}} {
    ctx->entity = entity;
    state_machine = new FSM::Instance{ctx};
    // HfsmLogger logger{};
    // state_machine->attachLogger((FSM::Instance::Logger*)&HFSM_DEBUG_LOGGER);
}

HumanoidMovementController::~HumanoidMovementController() {
    delete state_machine;
    delete ctx;
}

void HumanoidMovementController::OnGround::enter(Control& control) {
    auto context = control.context();

    context->dash_available = true;
}

void HumanoidMovementController::OnGround::update(FullControl& control) {
    auto& registry = Services::world_ptr()->registry();
    const auto& context = control.context();

    const auto& collider = registry.get<AabbCollider>(context->entity);
    if (!collider.is_grounded)
    {
        control.changeTo<InAir>();
        return;
    }

    Velocity* vel = registry.try_get<Velocity>(context->entity);
    Rotation* rot = registry.try_get<Rotation>(context->entity);

    vec3 dv = vec3_zero();

    vec3 right_vec = vec3_right(rot->yaw);
    vec3_normalize(&right_vec);

    vec3 forward = vec3_forward(rot->yaw, rot->pitch);
    vec3 forward_vec = vec3_new(forward.x, 0, forward.z);
    vec3_normalize(&forward_vec);

    f32 real_speed = 3 * 100;

    vec3_add_to(&dv, vec3_mul_scalar(forward_vec, context->z_input));
    vec3_add_to(&dv, vec3_mul_scalar(right_vec, context->x_input));

    if (context->x_input != 0 || context->z_input != 0)
    {
        vec3_normalize(&dv);
    }

    dv = vec3_mul_scalar(dv, real_speed * Services::Time::delta());

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

    vec3_add_to(&vel->vec, dv);
}

void HumanoidMovementController::OnGround::react(const HumanoidStateChangeEvent& action, EventControl& control)
{
    if (action.action_bits.has_bits(HumanoidActionBit::Jump))
    {
        control.changeTo<Jumping>();
        return;
    }

    if (action.action_bits.has_bits(HumanoidActionBit::Dash) && control.context()->dash_available)
        control.changeTo<Dashing>();
}

void HumanoidMovementController::InAir::enter(Control& control)
{

}

void HumanoidMovementController::InAir::update(FullControl& control) {
    const auto& context = control.context();
    const auto& registry = Services::world_ptr()->registry();

    const auto& collider = registry.get<AabbCollider>(context->entity);
    if (collider.is_grounded)
        control.changeTo<OnGround>();
}

void HumanoidMovementController::InAir::react(const HumanoidStateChangeEvent& action, EventControl& control)
{
    if (action.action_bits.has_bits(HumanoidActionBit::Dash) && control.context()->dash_available)
        control.changeTo<Dashing>();
}

void HumanoidMovementController::Dashing::enter(Control& control) {
    auto& context = control.context();
    const auto& registry = Services::world_ptr()->registry();

    context->dash_available = false;

    context->accumulator->action_bits.add_bits(HumanoidActionBit::Dash);

    const Rotation* rotation = registry.try_get<Rotation>(context->entity);

    if (context->x_input == 0 && context->z_input == 0)
    {
        vec3 forward = vec3_forward(rotation->yaw, rotation->pitch);
        context->dash_dir = forward;
    }
    else
    {
        // vec3 input_vec = vec3_new(context->x_input, 0, context->z_input);

        vec3 forward = vec3_forward(rotation->yaw, rotation->pitch);
        vec3 right = vec3_right(rotation->yaw);

        context->dash_dir = vec3_add(
            vec3_mul_scalar(right, context->x_input),
            vec3_mul_scalar(forward, context->z_input)
        );
        // context->dash_dir.y = 0;
        // context->action_bits.has_bits(HumanoidActionBit::Sneak)

        context->dash_dir = vec3_normalized(context->dash_dir);
    }
    GDF_StopwatchReset(context->dash_stopwatch);
}

void HumanoidMovementController::Dashing::update(FullControl& control) {
    const auto& context = control.context();
    auto& registry = Services::world_ptr()->registry();

    constexpr f32 dash_duration = 0.3f;
    constexpr f32 dash_power = 30.f;

    Velocity& vel = registry.get<Velocity>(context->entity);
    vel.vec = vec3_mul_scalar(context->dash_dir, dash_power);
    if (GDF_StopwatchElapsed(context->dash_stopwatch) >= dash_duration)
    {
        control.changeTo<Falling>();
        return;
    }

    const auto& collider = registry.get<AabbCollider>(context->entity);
    if (collider.is_grounded)
        control.changeTo<OnGround>();
}


void HumanoidMovementController::Dashing::exit(Control& control) {

}

void HumanoidMovementController::Jumping::enter(Control& control) {
    const auto& context = control.context();
    auto& registry = Services::world_ptr()->registry();

    Velocity& vel = registry.get<Velocity>(context->entity);

    context->accumulator->action_bits.add_bits(HumanoidActionBit::Jump);

    vec3 up = vec3_new(0, 1, 0);
    vec3_add_to(&vel.vec, vec3_mul_scalar(up, 7));
}

void HumanoidMovementController::Jumping::update(FullControl& control) {
    const auto& context = control.context();
    auto& registry = Services::world_ptr()->registry();

    Velocity& vel = registry.get<Velocity>(context->entity);
    if (vel.vec.y < -0.5)
        control.changeTo<Falling>();
}

void HumanoidMovementController::Jumping::exit(Control& control) {

}

void HumanoidMovementController::Falling::enter(Control& control) {

}

void HumanoidMovementController::Falling::update(FullControl& control) {

}

void HumanoidMovementController::Falling::exit(Control& control) {

}