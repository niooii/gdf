#include <client/graphics/renderer.h>

#include "events.h"

WorldRenderer::WorldRenderer(const GDF_VkRenderContext* vk_ctx)
{
    block_textures_init(vk_ctx, &this->block_textures);
    terrain_pipeline_init(vk_ctx, this);

    queued_remeshes.reserve(32);
    chunk_meshes.reserve(128);

    auto& events = GlobalEventManager::get_instance();
    events.subscribe<ChunkLoadEvent>([this](const auto& events)
    {
        for (const auto& event : events)
        {
            ivec3 cc = event.chunk_coord;
            LOG_DEBUG("loaded chunk at %d, %d, %d", cc.x, cc.y, cc.z);
            // chunk_meshes.insert_or_assign(cc, std::move(ChunkMesh{}));
        }
    });
}

WorldRenderer::~WorldRenderer()
{
    LOG_INFO("???");
    block_textures_destroy(&this->block_textures);
}
