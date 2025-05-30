#include <client/app.h>
#include <client/graphics/renderer.h>
#include <game/movement.h>
#include <game/physics/raycast.h>
#include <gdfe/input.h>
#include <gdfe/math/math.h>
#include <gdfe/os/thread.h>
#include <server/net.h>
#include <server/server.h>
#include "game/events/defs.h"

App APP{};

void App::join_world(const char* host, u16 port)
{
    client_world = new ClientWorld{ host, port };
    renderer->world_renderer.set_world(client_world->world_ptr());
}

void app_init()
{
    GDF_CameraCreateInfo camera_info = {
        .pos          = vec3_new(0.0f, 0.0f, 0.0f),
        .pitch        = DEG_TO_RAD(0.0f),
        .yaw          = DEG_TO_RAD(0.0f),
        .roll         = DEG_TO_RAD(0.0f),
        .aspect_ratio = 1.77f,
        .fov          = DEG_TO_RAD(75.f),
        .near_clip    = 0.1f,
        .far_clip     = 1000.0f,
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

GDF_BOOL app_update(const GDF_AppState* app_state, f64 dt, void* state)
{
    Services::Time::detail::_internal_dt = dt;
    App* app                             = &APP;
    // LOG_INFO("VEL: %f %f %f", player->base.vel.x, player->base.vel.y, player->base.vel.z);

    app->client_world->update(dt);

    Services::Events::flush();

    // LOG_DEBUG("pos: %f %f %f", collider->aabb.min.x, collider->aabb.min.y, collider->aabb.min.z);
    // LOG_DEBUG("vel: %f %f %f", player_comp->vel.x, player_comp->vel.y, player_comp->vel.z);
    return GDF_TRUE;
}
