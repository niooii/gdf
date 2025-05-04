#include <client/graphics/renderer.h>
#include <game/events/defs.h>
#include <prelude.h>

using namespace Services;

WorldRenderer::WorldRenderer(const GDF_VkRenderContext* vk_ctx)
{
    block_textures_init(vk_ctx, &this->block_textures);
    terrain_pipeline_init(vk_ctx, this);

    queued_remeshes.reserve(32);
    chunk_meshes.reserve(128);

    on_chunk_load = Events::subscribe<ChunkLoadEvent>(
        [this](const auto& event)
        {
            LOG_DEBUG("Recieved chunk load event!!");
            for (auto& load_info : event.loaded_chunks)
            {
                auto& cc = load_info.cc;
                LOG_DEBUG("loaded chunk at %d, %d, %d", cc.x, cc.y, cc.z);
                if (!chunk_meshes.contains(cc))
                    chunk_meshes[cc] = new ChunkMesh{ this->world, this->world->get_chunk(cc), cc };
            }
        });

    on_chunk_update = Events::subscribe<ChunkUpdateEvent>(
        [this](const auto& event)
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
    on_chunk_update->unsubscribe();
    on_chunk_load->unsubscribe();
    block_textures_destroy(&this->block_textures);
}
