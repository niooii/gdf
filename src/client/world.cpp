#include <client/world.h>
#include <client/app.h>
#include <game/entity/entity.h>

std::unique_ptr<HumanoidStateChangeEvent> ClientWorld::make_action_packet()
{
    /* Build an action event packet */
    ivec2 d;
    GDF_GetMouseDelta(&d);
    GDF_CameraAddYaw(APP.main_camera, DEG_TO_RAD(d.x * 0.1));
    GDF_CameraAddPitch(APP.main_camera, DEG_TO_RAD(d.y * 0.1));

    i8 z_input = 0;
    i8 x_input = 0;

    if (GDF_IsKeyDown(GDF_KEYCODE_W))
    {
        z_input++;
    }
    if (GDF_IsKeyDown(GDF_KEYCODE_S))
    {
        z_input--;
    }
    if (GDF_IsKeyDown(GDF_KEYCODE_A))
    {
        x_input--;
    }
    if (GDF_IsKeyDown(GDF_KEYCODE_D))
    {
        x_input++;
    }

    auto action_event = Services::Events::create_event<HumanoidStateChangeEvent>();
    action_event->x_input = x_input;
    action_event->z_input = z_input;

    action_event->action_bits.mask = 0;

    if (GDF_IsKeyDown(GDF_KEYCODE_SPACE))
        action_event->add_bits(HumanoidActionBit::Jump);

    if (GDF_IsKeyDown(GDF_KEYCODE_SHIFT))
        action_event->add_bits(HumanoidActionBit::Sneak);

    if (GDF_IsKeyPressed(GDF_KEYCODE_Q))
        action_event->add_bits(HumanoidActionBit::Dash);

    if (GDF_IsButtonDown(GDF_MBUTTON_LEFT))
        action_event->add_bits(HumanoidActionBit::Attack);

    if (GDF_IsButtonDown(GDF_MBUTTON_RIGHT))
        action_event->add_bits(HumanoidActionBit::Use);

    action_event->pitch = GDF_CameraGetPitch(APP.main_camera);
    action_event->yaw = GDF_CameraGetYaw(APP.main_camera);

    return std::move(action_event);
}

void ClientWorld::update(f32 dt)
{
    // gather the player input
    // this doesnt really fit in this function well does it idk - maybe
    // move one level higher (the main app loop) or i might be stupid
    auto player_action_event{make_action_packet()};

    // TODO! simulate the inputs on client side - this is a rough sketch of how to do it

    // on client there should only be one controller
    auto& hum = world_->simulated_humanoids()[0];
    // upd the rotation component - this will be done on the server side after recieving
    // the action event
    Components::Rotation* rotation = world_->registry().try_get<Components::Rotation>(main_player_);
    rotation->pitch = player_action_event->pitch;
    rotation->yaw = player_action_event->yaw;
    hum.process_action(*player_action_event);

    // Send the player input to the server
    // server_con_.send(std::move(player_action_event));

    world_->update(dt);
    server_con_.dispatch_incoming();

    // update camera pos to be center of "head"
    const auto* collider = world_->registry().try_get<Components::AabbCollider>(main_player_);
    vec3 camera_pos = vec3_new(
        (collider->aabb.min.x + collider->aabb.max.x) / 2.0,
        collider->aabb.max.y - 0.25,
        (collider->aabb.min.z + collider->aabb.max.z) / 2.0
    );
    GDF_CameraSetPosition(
        APP.main_camera,
        camera_pos
    );
}