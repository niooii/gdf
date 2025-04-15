#include <client/app.h>
#include <gdfe/input.h>
#include <gdfe/math/math.h>
#include <gdfe/math/math.h>
#include <client/graphics/renderer.h>
#include <game/movement.h>
#include <game/physics/raycast.h>
#include <gdfe/os/thread.h>
#include <server/net.h>
#include <server/server.h>
#include "game/events/defs.h"

App APP{};

void App::join_world(const char* host, u16 port)
{
    client_world = new ClientWorld{host, port};
    renderer->world_renderer.set_world(client_world->world_ptr());
}

void app_init()
{
    GDF_CameraCreateInfo camera_info = {
        .pos = vec3_new(0.0f, 0.0f, 0.0f),
        .pitch = DEG_TO_RAD(0.0f),
        .yaw = DEG_TO_RAD(0.0f),
        .roll = DEG_TO_RAD(0.0f),
        .aspect_ratio = 1.77f,
        .fov = DEG_TO_RAD(75.f),
        .near_clip = 0.1f,
        .far_clip = 1000.0f,
    };
    APP.main_camera = GDF_CameraCreate(&camera_info);
    GDF_CameraSetGlobalAxis(APP.main_camera, vec3_new(0, 1, 0));
    GDF_CameraConstrainPitch(APP.main_camera, -DEG_TO_RAD(89.9), DEG_TO_RAD(89.9));

    // TODO! uncomment later, the game will be initilaized in world state for now.
    // APP.current_screen = GDF_GAME_SCREEN_MAIN_MENU;
    // APP.current_screen_type = GDF_GAME_SCREENTYPE_GUI_MENU;
    // APP.main_player = NULL;
    // APP.world = NULL;

    // APP.main_player = NULL;
    // WorldCreateInfo world_info = {
    // };
    // APP.world = new World(world_info);
    // player = APP.world->create_humanoid();
    // physics_add_entity(APP.world->physics_, &player->base);
    // collider->aabb.min = vec3_new(-0.375, 0, -0.375);
    // collider->aabb.max = vec3_new(0.375, 1.8, 0.375);
    // aabb_translate(&collider->aabb, vec3_new(1, 5, 1));
    // player->base.health = 100;
    // player->base.damagable = GDF_TRUE;
}

// TODO! remove this from here prob
// temporary input controls. will switch to registering different input handlers
// later.
// void game_handle_input(App* game, f64 dt)
// {
//     GDF_Camera camera = APP.main_camera;
//     auto player = game->client_world->main_player;
//     {
//         ivec2 d;
//         GDF_GetMouseDelta(&d);
//         GDF_CameraAddYaw(camera, DEG_TO_RAD(d.x * 0.1));
//         GDF_CameraAddPitch(camera, DEG_TO_RAD(d.y * 0.1));
//
//         // wrap around yaw
//         // if (camera->yaw > 180)
//         // {
//         //     camera->yaw = -180 + (camera->yaw - 180);
//         // }
//         // else if (camera->yaw < -180)
//         // {
//         //     camera->yaw = 180 + (camera->yaw + 180);
//         // }
//         // TODO! weird behavior when not clamped: when pitch passes -90 or 90, scene flips??
//         // camera->pitch = CLAMP(camera->pitch, -89.9, 89.9);
//     }
//
//     if (GDF_IsKeyDown(GDF_KEYCODE_E))
//     {
//         GDF_CameraAddRoll(camera, dt);
//     }
//     if (GDF_IsKeyDown(GDF_KEYCODE_R))
//     {
//         GDF_CameraAddRoll(camera, -dt);
//     }
//
//     vec3 camera_forward, camera_right, camera_up;
//     GDF_CameraOrientation(camera, &camera_forward, &camera_right, &camera_up);
//
//     // LOG_DEBUG("Camera forward: %f, %f, %f", camera_forward.x, camera_forward.y, camera_forward.z);
//     // LOG_DEBUG("Camera right: %f, %f, %f", camera_right.x, camera_right.y, camera_right.z);
//     // LOG_DEBUG("Camera up: %f, %f, %f", camera_up.x, camera_up.y, camera_up.z);
//
//     i8 z_input = 0;
//     i8 x_input = 0;
//     if (GDF_IsKeyDown(GDF_KEYCODE_W))
//     {
//         z_input++;
//     }
//     if (GDF_IsKeyDown(GDF_KEYCODE_S))
//     {
//         z_input--;
//     }
//     if (GDF_IsKeyDown(GDF_KEYCODE_A))
//     {
//         x_input--;
//     }
//     if (GDF_IsKeyDown(GDF_KEYCODE_D))
//     {
//         x_input++;
//     }
//
//     auto action_event = Services::Events::create_event<HumanoidActionEvent>();
//     action_event->x_input = x_input;
//     action_event->z_input = z_input;
//
//     if (GDF_IsKeyDown(GDF_KEYCODE_SPACE))
//         action_event->add_bits(HumanoidActionBit::Jump);
//
//     if (GDF_IsKeyDown(GDF_KEYCODE_SHIFT))
//         action_event->add_bits(HumanoidActionBit::Sneak);
//
//     if (GDF_IsKeyDown(GDF_KEYCODE_Q))
//         action_event->add_bits(HumanoidActionBit::Dash);
//
//     game->client_world->send_action(std::move(action_event));
//
//     World* world = game->client_world->world;
//     Components::AabbCollider* collider = world->registry().try_get<Components::AabbCollider>(player);
//
//     // if (GDF_IsKeyDown(GDF_KEYCODE_SPACE) && player->base.grounded)
//     // {
//     //     // jump(player, global_up, 1);
//     //     jumped = GDF_TRUE;
//     // }
//
//     f32 move_speed = 1;
//     if (GDF_IsKeyDown(GDF_KEYCODE_LCONTROL))
//     {
//         move_speed = 3.5;
//     }
//     // player_apply_movement(
//     //     player,
//     //     x_input,
//     //     z_input,
//     //     &camera_forward,
//     //     &camera_right,
//     //     dt,
//     //     jumped,
//     //     move_speed
//     // );
//
//     // cam is in center of players head
//     vec3 camera_pos = vec3_new(
//         (collider->aabb.min.x + collider->aabb.max.x) / 2.0,
//         collider->aabb.max.y - 0.25,
//         (collider->aabb.min.z + collider->aabb.max.z) / 2.0
//     );
//     GDF_CameraSetPosition(
//         camera,
//         camera_pos
//     );
//
//     if (GDF_IsKeyDown(GDF_KEYCODE_LSHIFT))
//     {
//         // player->base.vel.y = -move_speed;
//     }
//     // if (GDF_IsKeyPressed(GDF_KEYCODE_V))
//     // {
//     //     GDF_RendererState* renderer = renderer_get_instance();
//     //     renderer->render_mode = !renderer->render_mode;
//     // }
//     if (GDF_IsButtonDown(GDF_MBUTTON_LEFT))
//     {
//         RaycastInfo raycast_info = raycast_info_new(
//             world,
//             camera_pos,
//             camera_forward,
//             4
//         );
//         RaycastBlockHitInfo result;
//         raycast_blocks(&raycast_info, &result);
//         if (result.status == RAYCAST_STATUS_HIT)
//         {
//             LOG_DEBUG("destroying block..");
//             BlockCreateInfo info = {
//                 .type = BLOCK_TYPE_Air,
//                 .world_pos = result.block_world_pos
//             };
//             APP.client_world->world->set_block(info);
//         }
//     }
//     else if (GDF_IsButtonDown(GDF_MBUTTON_RIGHT))
//     {
//         RaycastInfo raycast_info = raycast_info_new(
//             world,
//             camera_pos,
//             camera_forward,
//             4
//         );
//         RaycastBlockHitInfo result;
//         raycast_blocks(&raycast_info, &result);
//         if (result.status == RAYCAST_STATUS_HIT)
//         {
//             i8 x_dif=0, y_dif=0, z_dif=0;
//             switch (result.hit_face)
//             {
//                 case BLOCK_FACE_BOT:
//                     y_dif = -1;
//                     break;
//                 case BLOCK_FACE_TOP:
//                     y_dif = 1;
//                     break;
//                 case BLOCK_FACE_BACK:
//                     z_dif = -1;
//                     break;
//                 case BLOCK_FACE_FRONT:
//                     z_dif = 1;
//                     break;
//                 case BLOCK_FACE_LEFT:
//                     x_dif = -1;
//                     break;
//                 case BLOCK_FACE_RIGHT:
//                     x_dif = 1;
//                     break;
//             }
//             vec3 place_pos = vec3_new(
//                 result.block_world_pos.x + x_dif,
//                 result.block_world_pos.y + y_dif,
//                 result.block_world_pos.z + z_dif
//             );
//             AxisAlignedBoundingBox block = block_get_aabb(place_pos);
//             if (!aabb_intersects(&block, &collider->aabb)) {
//                 BlockCreateInfo info = {
//                     .type = BLOCK_TYPE_WoodPlank,
//                     .world_pos = place_pos
//                 };
//                 APP.client_world->world->set_block(info);
//             }
//         }
//     }
// }

GDF_BOOL app_update(const GDF_AppState* app_state, f64 dt, void* state)
{
    App* app = (App*)state;
    // LOG_INFO("VEL: %f %f %f", player->base.vel.x, player->base.vel.y, player->base.vel.z);

    app->client_world->update(dt);

    Services::Events::flush();

    // LOG_DEBUG("pos: %f %f %f", collider->aabb.min.x, collider->aabb.min.y, collider->aabb.min.z);
    // LOG_DEBUG("vel: %f %f %f", player_comp->vel.x, player_comp->vel.y, player_comp->vel.z);
    return GDF_TRUE;
}
