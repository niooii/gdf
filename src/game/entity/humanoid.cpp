#include <game/entity/humanoid.h>
#include <game/physics/engine.h>
#include <game/world.h>

using namespace Systems;
using namespace Components;

/* State function definitions */

HumanoidMovementController::HumanoidMovementController(ecs::Entity entity) {
    MovementContext ctx{};
    ctx.entity = entity;
    state_machine = new FSM::Instance{std::move(ctx)};
    // HfsmLogger logger{};
    // state_machine->attachLogger((FSM::Instance::Logger*)&HFSM_DEBUG_LOGGER);
}

HumanoidMovementController::~HumanoidMovementController() {
    delete state_machine;
}

void HumanoidMovementController::OnGround::enter(Control& control) {
    auto& context = control.context();
    // const auto& registry = APP.client_world->world_ptr()->registry();
    LOG_CALL

    context.dash_available = true;
}

void HumanoidMovementController::OnGround::update(FullControl& control) {
    const auto& registry = Services::world_ptr()->registry();
    const auto& context = control.context();

    const auto& collider = registry.get<AabbCollider>(context.entity);
    if (!collider.is_grounded)
        control.changeTo<InAir>();
}

void HumanoidMovementController::OnGround::react(const HumanoidActionEvent& action, EventControl& control)
{
    if (action.has_bits(HumanoidActionBit::Jump))
    {
        control.changeTo<Jumping>();
        return;
    }

    if (action.has_bits(HumanoidActionBit::Dash) && control.context().dash_available)
        control.changeTo<Dashing>();
}

void HumanoidMovementController::InAir::enter(Control& control)
{

}

void HumanoidMovementController::InAir::update(FullControl& control) {
    const auto& context = control.context();
    const auto& registry = Services::world_ptr()->registry();

    const auto& collider = registry.get<AabbCollider>(context.entity);
    if (collider.is_grounded)
        control.changeTo<OnGround>();
}

void HumanoidMovementController::InAir::react(const HumanoidActionEvent& action, EventControl& control)
{
    if (action.has_bits(HumanoidActionBit::Dash) && control.context().dash_available)
        control.changeTo<Dashing>();
}

void HumanoidMovementController::Dashing::enter(Control& control) {
    auto& context = control.context();
    LOG_CALL

    context.dash_available = false;
    vec3 forward, right, up;
    // TODO! calculate with pitch and yaw
    // GDF_CameraOrientation(APP.main_camera, &forward, &right, &up);
    context.dash_dir = forward;
    GDF_StopwatchReset(context.dash_stopwatch);
}

void HumanoidMovementController::Dashing::update(FullControl& control) {
    const auto& context = control.context();
    auto& registry = Services::world_ptr()->registry();

    constexpr f32 dash_duration = 0.3f;
    constexpr f32 dash_power = 30.f;

    Velocity& vel = registry.get<Velocity>(context.entity);
    vel.vec = vec3_mul_scalar(context.dash_dir, dash_power);
    if (GDF_StopwatchElapsed(context.dash_stopwatch) >= dash_duration)
    {
        control.changeTo<Falling>();
        return;
    }

    const auto& collider = registry.get<AabbCollider>(context.entity);
    if (collider.is_grounded)
        control.changeTo<OnGround>();
}


void HumanoidMovementController::Dashing::exit(Control& control) {

}

void HumanoidMovementController::Jumping::enter(Control& control) {
    const auto& context = control.context();
    auto& registry = Services::world_ptr()->registry();

    Velocity& vel = registry.get<Velocity>(context.entity);
    LOG_CALL

    // vec3 up = GDF_CameraGetGlobalAxis(APP.main_camera);
    vec3 up = vec3_new(0, 1, 0);
    vec3_add_to(&vel.vec, vec3_mul_scalar(up, 7));
}

void HumanoidMovementController::Jumping::update(FullControl& control) {
    const auto& context = control.context();
    auto& registry = Services::world_ptr()->registry();

    Velocity& vel = registry.get<Velocity>(context.entity);
    if (vel.vec.y < -0.5)
        control.changeTo<Falling>();
}

void HumanoidMovementController::Jumping::exit(Control& control) {

}

void HumanoidMovementController::Falling::enter(Control& control) {
    LOG_CALL
}

void HumanoidMovementController::Falling::update(FullControl& control) {

}

void HumanoidMovementController::Falling::exit(Control& control) {

}