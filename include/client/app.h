#pragma once

#include <gdfe/core.h>
#include <game/world.h>
#include <gdfe/camera.h>

#include <gdfe/gdfe.h>

#include <client/graphics/renderer.h>

#include "net.h"

// TODO! replace with UIManager to push and pop screens or smthing
// typedef enum GDF_GAME_SCREEN {
//     GDF_GAME_SCREEN_MAIN_MENU,
//     GDF_GAME_SCREEN_IN_WORLD,
// } GDF_GAME_SCREEN;
//
// typedef enum GDF_GAME_SCREENTYPE {
//     GDF_GAME_SCREENTYPE_GUI_MENU,
//     GDF_GAME_SCREENTYPE_WORLD,
//     GDF_GAME_SCREENTYPE_WORLD_GUI_MENU,
// } GDF_GAME_SCREENTYPE;

using EntityId = u64;

struct ClientWorld {
    World world;
    ServerConnection server_con;
    EntityId main_player;

    FORCEINLINE void update(f32 dt)
    {
        world.update(dt);
        server_con.dispatch_incoming();
    }
};

struct App {
    // This will be NULL if the player is not in a world.
    ClientWorld* world = NULL;

    // HumanoidEntity* main_player;
    // GDF_GAME_SCREEN current_screen;
    // GDF_GAME_SCREENTYPE current_screen_type;

    GDF_Camera main_camera;

    void join_world(const char* host, u16 port);
};

// The global app state.
extern App APP;

void app_init();
void app_destroy();
GDF_BOOL app_update(const GDF_AppState* app_state, f64 delta_time, void* state);

