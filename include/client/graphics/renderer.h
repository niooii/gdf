#pragma once

#include <gdfe/gdfe.h>
#include <unordered_dense.h>

#include "chunkmesh.h"
#include "pipelines.h"

typedef struct WorldRenderer {
    World* world;
    ankerl::unordered_dense::set<ivec3> queued_remeshes{};
    ankerl::unordered_dense::map<ivec3, ChunkMesh*> chunk_meshes{};

    terrain_pipeline terrain_pipeline;
    block_textures block_textures;

    // Event subscriptions
    std::unique_ptr<Subscription> on_chunk_load;
    std::unique_ptr<Subscription> on_chunk_update;

    // Created once when game is loaded
    WorldRenderer(const GDF_VkRenderContext* vk_ctx);
    ~WorldRenderer();

    // should be called when entering a new world
    FORCEINLINE void set_world(World* world) { this->world = world; }
} WorldRenderer;

typedef struct GameRenderer {
    // contains everything needed to render a world
    WorldRenderer world_renderer;

    // TODO!
    // need states for player in menu / in game
    GameRenderer(const GDF_VkRenderContext* vk_ctx);
    ~GameRenderer();
} GameRenderer;

GDF_BOOL renderer_init(const GDF_VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);
GDF_BOOL renderer_destroy(const GDF_VkRenderContext* vulkan_ctx, const GDF_AppState* app_state, void* state);
GDF_BOOL renderer_draw(const GDF_VkRenderContext* vulkan_ctx, GDF_RENDER_MODE mode, const GDF_AppState* app_state, void* state);