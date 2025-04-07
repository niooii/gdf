#pragma once

#include <gdfe/core.h>
#include <game/world.h>
#include <gdfe/camera.h>

#include <gdfe/gdfe.h>

#include <client/graphics/renderer.h>

typedef enum GDF_GAME_SCREEN {
    GDF_GAME_SCREEN_MAIN_MENU,
    GDF_GAME_SCREEN_IN_WORLD,
} GDF_GAME_SCREEN;

typedef enum GDF_GAME_SCREENTYPE {
    GDF_GAME_SCREENTYPE_GUI_MENU,
    GDF_GAME_SCREENTYPE_WORLD,
    GDF_GAME_SCREENTYPE_WORLD_GUI_MENU,
} GDF_GAME_SCREENTYPE;

typedef struct AppState {
    World* world;
    // HumanoidEntity* main_player;
    GDF_GAME_SCREEN current_screen;
    GDF_GAME_SCREENTYPE current_screen_type;

    GameRenderer* renderer;

    GDF_Camera main_camera;
} AppState;

AppState* app_init(AppState* game);
void app_destroy();
GDF_BOOL app_update(const GDF_AppState* app_state, f64 delta_time, void* state);
