#include <client/graphics/renderer.h>
#include <game/prelude.h>
#include <game/events.h>

WorldRenderer::WorldRenderer(const GDF_VkRenderContext* vk_ctx, World* world)
    : world{world}
{
    block_textures_init(vk_ctx, &this->block_textures);
    terrain_pipeline_init(vk_ctx, this);

    queued_remeshes.reserve(32);
    chunk_meshes.reserve(128);

    auto& event_manager = EventManager::get_instance();

    event_manager.subscribe<ChunkLoadEvent>([this](const auto& event)
    {
        for (auto& cc : event.loaded_chunks) {
            LOG_INFO("loaded chunk at %d, %d, %d", cc.x, cc.y, cc.z);
            if (!chunk_meshes.contains(cc))
                chunk_meshes[cc] = new ChunkMesh{this->world, this->world->get_chunk(cc), cc};
        }
    });

    event_manager.subscribe<ChunkUpdateEvent>([this](const auto& event)
    {
        ivec3 cc = event.chunk_coord;

        if (chunk_meshes.contains(cc))
        {
            this->chunk_meshes[cc]->mesh();
            // if ()
        }
    });
}

WorldRenderer::~WorldRenderer()
{
    LOG_INFO("???");
    block_textures_destroy(&this->block_textures);
}
