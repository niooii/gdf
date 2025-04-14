#include <game/entity/humanoid.h>
#include <client/app.h>

using namespace Components;

/* State function definitions */

MovementControl::MovementControl(ecs::Entity entity) {
    MovementContext ctx{};
    ctx.entity = entity;
    state_machine = new FSM::Instance{std::move(ctx)};
    // HfsmLogger logger{};
    // state_machine->attachLogger((FSM::Instance::Logger*)&HFSM_DEBUG_LOGGER);
}

void MovementControl::OnGround::enter(Control& control) {
    auto& context = control.context();
    const auto& registry = APP.client_world->world->registry();
    LOG_CALL

    context.dash_available = true;

    auto& collider = registry.get<AabbCollider>(context.entity);

    LOG_DEBUG("GROUNDED STATE: %d", collider.is_grounded);
}

void MovementControl::OnGround::update(FullControl& control) {
    const auto& registry = APP.client_world->world->registry();
    const auto& context = control.context();

    if (GDF_IsKeyPressed(GDF_KEYCODE_Q) && control.context().dash_available)
    {
        LOG_TRACE("q is pressed and can dash");
        control.changeTo<Dashing>();
    }

    if (GDF_IsKeyDown(GDF_KEYCODE_SPACE))
        control.changeTo<Jumping>();

    const auto& collider = registry.get<AabbCollider>(context.entity);
    if (!collider.is_grounded)
        control.changeTo<InAir>();
}

void MovementControl::InAir::enter(Control& control)
{
}


void MovementControl::InAir::update(FullControl& control) {
    const auto& context = control.context();
    const auto& registry = APP.client_world->world->registry();

    LOG_CALL
    if (GDF_IsKeyPressed(GDF_KEYCODE_Q) && control.context().dash_available)
    {
        LOG_TRACE("[AIR] q is pressed and can dash");
        control.changeTo<Dashing>();
    }

    const auto& collider = registry.get<AabbCollider>(context.entity);
    if (collider.is_grounded)
        control.changeTo<OnGround>();
}

void MovementControl::Dashing::enter(Control& control) {
    auto& context = control.context();
    LOG_CALL

    context.dash_available = false;
    vec3 forward, right, up;
    GDF_CameraOrientation(APP.main_camera, &forward, &right, &up);
    context.dash_dir = forward;
    GDF_StopwatchReset(context.dash_stopwatch);
}

void MovementControl::Dashing::update(FullControl& control) {
    const auto& context = control.context();
    auto& registry = APP.client_world->world->registry();

    constexpr f32 dash_duration = 0.3f;
    constexpr f32 dash_power = 30.f;

    Velocity& vel = registry.get<Velocity>(context.entity);
    vel.vec = vec3_mul_scalar(context.dash_dir, dash_power);
    if (GDF_StopwatchElapsed(context.dash_stopwatch) >= dash_duration)
        control.changeTo<Falling>();
}


void MovementControl::Dashing::exit(Control& control) {

}

void MovementControl::Jumping::enter(Control& control) {
    const auto& context = control.context();
    auto& registry = APP.client_world->world->registry();

    Velocity& vel = registry.get<Velocity>(context.entity);
    LOG_CALL

    vec3 up = GDF_CameraGetGlobalAxis(APP.main_camera);
    vec3_add_to(&vel.vec, vec3_mul_scalar(up, 7));
}

void MovementControl::Jumping::update(FullControl& control) {
    const auto& context = control.context();
    auto& registry = APP.client_world->world->registry();

    Velocity& vel = registry.get<Velocity>(context.entity);
    if (vel.vec.y < -0.5)
        control.changeTo<Falling>();
}

void MovementControl::Jumping::exit(Control& control) {

}

void MovementControl::Falling::enter(Control& control) {
    LOG_CALL
}

void MovementControl::Falling::update(FullControl& control) {

}

void MovementControl::Falling::exit(Control& control) {

}