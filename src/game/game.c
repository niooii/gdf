#include <game/game.h>
#include <input.h>
#include <math/math.h>
#include <math/math.h>
#include <graphics/renderer.h>
#include <game/movement.h>
#include <physics/raycast.h>

static HumanoidEntity* player;

GameState* game_init()
{
    GameState* game = GDF_Malloc(sizeof(GameState), GDF_MEMTAG_GAME);
    GDF_CameraCreateInfo camera_info = {
        .pos = vec3_new(0.0f, 0.0f, 0.0f),
        .pitch = 0.0f,
        .yaw = 0.0f,
        .roll = 0.0f,
        .aspect_ratio = 1.77f,
        .fov = DEG_TO_RAD(45.f),
        .near_clip = 0.1f,
        .far_clip = 1000.0f,
    };
    game->main_camera = GDF_CameraCreate(&camera_info);

    // TODO! uncomment later, the game will be initilaized in world state for now.
    // game->current_screen = GDF_GAME_SCREEN_MAIN_MENU;
    // game->current_screen_type = GDF_GAME_SCREENTYPE_GUI_MENU;
    // game->main_player = NULL;
    // game->world = NULL;

    game->current_screen = GDF_GAME_SCREEN_IN_WORLD;
    game->current_screen_type = GDF_GAME_SCREENTYPE_WORLD;
    // game->main_player = NULL;
    game->world = GDF_Malloc(sizeof(World), GDF_MEMTAG_GAME);
    WorldCreateInfo world_info = {
        .chunk_simulate_distance = 16,
        .ticks_per_sec = 20,
    };
    world_create(game->world, &world_info);
    player = world_create_humanoid(game->world);
    physics_add_entity(game->world->physics, &player->base);
    player->base.aabb.min = vec3_new(-0.375, 0, -0.375);
    player->base.aabb.max = vec3_new(0.375, 1.8, 0.375);
    aabb_translate(&player->base.aabb, vec3_new(1, 5, 1));
    player->base.health = 100;
    player->base.damagable = true;

    return game;
}

// TODO! remove this from here prob
void game_handle_input(GameState* game, f64 dt)
{
    GDF_Camera camera = game->main_camera;
    {
        ivec2 d;
        GDF_GetMouseDelta(&d);
        GDF_CameraAddYaw(camera, d.x * 0.1);
        GDF_CameraAddPitch(camera, d.y * 0.1);

        // wrap around yaw
        // if (camera->yaw > 180)
        // {
        //     camera->yaw = -180 + (camera->yaw - 180);
        // }
        // else if (camera->yaw < -180)
        // {
        //     camera->yaw = 180 + (camera->yaw + 180);
        // }
        // TODO! weird behavior when not clamped: when pitch passes -90 or 90, scene flips??
        // camera->pitch = CLAMP(camera->pitch, -89.9, 89.9);
    }

    vec3 camera_forward, camera_right, camera_up;
    GDF_CameraOrientation(camera, &camera_forward, &camera_right, &camera_up);

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
    if (GDF_IsKeyPressed(GDF_KEYCODE_Q))
    {
        dash(player, 1.f, camera_forward);
    }
    bool jumped = false;
    if (GDF_IsKeyDown(GDF_KEYCODE_SPACE) && player->base.grounded)
    {
        jump(player, 1);
        jumped = true;
    }

    f32 move_speed = 1;
    if (GDF_IsKeyDown(GDF_KEYCODE_LCONTROL))
    {
        move_speed = 1.5;
    }
    player_apply_movement(
        player,
        x_input,
        z_input,
        &camera_forward,
        &camera_right,
        dt,
        jumped,
        move_speed
    );

    // cam is in center of players head
    vec3 camera_pos = vec3_new(
        (player->base.aabb.min.x + player->base.aabb.max.x) / 2.0,
        player->base.aabb.max.y - 0.5,
        (player->base.aabb.min.z + player->base.aabb.max.z) / 2.0
    );
    GDF_CameraSetPosition(
        camera,
        camera_pos
    );

    if (GDF_IsKeyDown(GDF_KEYCODE_LSHIFT))
    {
        player->base.vel.y = -move_speed;
    }
    // if (GDF_IsKeyPressed(GDF_KEYCODE_V))
    // {
    //     GDF_RendererState* renderer = renderer_get_instance();
    //     renderer->render_mode = !renderer->render_mode;
    // }
    if (GDF_IsButtonDown(GDF_MBUTTON_LEFT))
    {
        RaycastInfo raycast_info = raycast_info_new(
            game->world,
            camera_pos,
            camera_forward,
            4
        );
        RaycastBlockHitInfo result;
        raycast_blocks(&raycast_info, &result);
        if (result.status == RAYCAST_STATUS_HIT)
        {
            LOG_DEBUG("destroying block..");
            world_destroy_block(game->world, result.block_world_pos, NULL);
        }
    }
    else if (GDF_IsButtonDown(GDF_MBUTTON_RIGHT))
    {
        RaycastInfo raycast_info = raycast_info_new(
            game->world,
            camera_pos,
            camera_forward,
            4
        );
        RaycastBlockHitInfo result;
        raycast_blocks(&raycast_info, &result);
        if (result.status == RAYCAST_STATUS_HIT)
        {
            i8 x_dif=0, y_dif=0, z_dif=0;
            switch (result.hit_face)
            {
                case BLOCK_FACE_BOT:
                    y_dif = -1;
                    break;
                case BLOCK_FACE_TOP:
                    y_dif = 1;
                    break;
                case BLOCK_FACE_BACK:
                    z_dif = -1;
                    break;
                case BLOCK_FACE_FRONT:
                    z_dif = 1;
                    break;
                case BLOCK_FACE_LEFT:
                    x_dif = -1;
                    break;
                case BLOCK_FACE_RIGHT:
                    x_dif = 1;
                    break;
            }
            vec3 place_pos = vec3_new(
                result.block_world_pos.x + x_dif,
                result.block_world_pos.y + y_dif,
                result.block_world_pos.z + z_dif
            );
            AxisAlignedBoundingBox block = block_get_aabb(place_pos);
            if (!aabb_intersects(&block, &player->base.aabb)) {
                BlockCreateInfo info = {
                    .type = BLOCK_TYPE_Grass,
                    .world_pos = place_pos
                };
                world_set_block(game->world, &info);
            }
        }
    }
}

bool game_update(const GDF_AppState* app_state, f64 dt, void* state)
{
    LOG_INFO("DT: %lf", dt);
    GameState* game = (GameState*)state;
    // LOG_INFO("VEL: %f %f %f", player->base.vel.x, player->base.vel.y, player->base.vel.z);
    game_handle_input(game, dt);
    world_update(game->world, dt);

    LOG_DEBUG("pos: %f %f %f", player->base.aabb.min.x, player->base.aabb.min.y, player->base.aabb.min.z);
    // LOG_DEBUG("vel: %f %f %f", player_comp->vel.x, player_comp->vel.y, player_comp->vel.z);
    return true;
}
