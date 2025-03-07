#pragma once

#include <core.h>
#include <game/world.h>
#include <camera.h>

typedef enum GDF_GAME_SCREEN {
    GDF_GAME_SCREEN_MAIN_MENU,
    GDF_GAME_SCREEN_IN_WORLD,
} GDF_GAME_SCREEN;

typedef enum GDF_GAME_SCREENTYPE {
    GDF_GAME_SCREENTYPE_GUI_MENU,
    GDF_GAME_SCREENTYPE_WORLD,
    GDF_GAME_SCREENTYPE_WORLD_GUI_MENU,
} GDF_GAME_SCREENTYPE;

typedef struct GDF_Game {
    World* world;
    // HumanoidEntity* main_player;
    GDF_GAME_SCREEN current_screen;
    GDF_GAME_SCREENTYPE current_screen_type;

    GDF_Camera* main_camera;
} GDF_Game;

bool GDF_GAME_Init();
bool GDF_GAME_Update(f32 dt);
GDF_Game* GDF_GAME_GetInstance();