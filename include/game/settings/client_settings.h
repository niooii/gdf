#pragma once

#include <gdfe/../../../gdfe/include/pub/core.h>
#include <gdfe/movement.h>
#include <gdfe/keybinds.h>
#include <gdfe/graphics.h>
#include <gdfe/mouse.h>

typedef struct GDF_Settings {
    GDF_Keybinds keybinds;
    GDF_MouseSettings mouse;
    GDF_MovementSettings movement;
    GDF_GraphicSettings graphics;
} GDF_Settings;

// search for settings.gdf or something like taht idk
GDF_Settings* GDF_LoadSettings();
void GDF_SaveSettings();