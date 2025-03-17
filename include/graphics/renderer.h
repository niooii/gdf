#pragma once

#include <gdfe/gdfe.h>
#include <unordered_dense.h>

#include "chunkmesh.h"
#include "pipelines.h"

typedef struct WorldRenderer {
    World* world;
    ankerl::unordered_dense::set<ivec3> queued_remeshes{};
    ankerl::unordered_dense::map<ivec3, ChunkMesh> chunk_meshes{};

    terrain_pipeline terrain_pipeline;
    block_textures block_textures;

    WorldRenderer(const GDF_VkRenderContext* vk_ctx);
    ~WorldRenderer();
} WorldRenderer;

typedef struct GameRenderer {
    // contains everything needed to render a world
    WorldRenderer world_renderer;

    // TODO! this shouldnt accept the world at first.
    // need states for player in menu / in game
    GameRenderer(const GDF_VkRenderContext* vk_ctx, World* world);
    ~GameRenderer();
} GameRenderer;

GDF_BOOL renderer_init(const GDF_VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);
GDF_BOOL renderer_destroy(const GDF_VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);
GDF_BOOL renderer_draw(const GDF_VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);