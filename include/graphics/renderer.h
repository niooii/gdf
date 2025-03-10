#pragma once

#include <graphics/renderer.h>
#include <gdfe.h>

bool renderer_init(const VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);
bool renderer_destroy(const VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);
bool renderer_draw(const VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);