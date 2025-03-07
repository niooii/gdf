#pragma once

#include <render/renderer.h>

// TODO! could update this to be its own thing in the game folder
// could separate like pipelines, specific stuff out of the core renderer code
// and add a function to add a render hook `bool (*hook)(cmd_buffer)` before the command buffer finishes in the
// core render code.  Thats where the actual game rendering happens. WAIT TAHTS SO FIRE HOLD UP

// ALSO TODO! make a separate pipeline for misc rendering like random lines and polygon for debugging and stuff.  
bool vk_game_renderer_init(VkRenderContext* context, Renderer* backend);
bool vk_game_renderer_draw(VkRenderContext* context, Renderer* backend, u8 resource_idx, f32 dt);
