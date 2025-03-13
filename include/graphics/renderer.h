#pragma once

#include <gdfe/graphics/renderer.h>
#include <gdfe/../../gdfe/include/gdfe/gdfe.h>

typedef struct GameRenderer {

} GameRenderer;

GDF_BOOL renderer_init(const GDF_VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);
GDF_BOOL renderer_destroy(const GDF_VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);
GDF_BOOL renderer_draw(const GDF_VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);