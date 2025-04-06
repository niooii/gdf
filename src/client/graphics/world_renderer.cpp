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

    auto& events = EventManager::get_instance();

    events.subscribe<ChunkLoadEvent>([this](const auto& events)
    {
        for (const auto& event : events)
        {
            ivec3 cc = event.chunk_coord;
            LOG_DEBUG("loaded chunk at %d, %d, %d", cc.x, cc.y, cc.z);
            if (chunk_meshes.contains(cc))
            {
                continue;
            }
            chunk_meshes[cc] = new ChunkMesh{this->world, this->world->get_chunk(cc), cc};
        }
    });

    events.subscribe<ChunkUpdateEvent>([this](const auto& events)
    {
        ankerl::unordered_dense::set<ivec3> to_remesh{};

        for (const auto& event : events)
        {
            ivec3 cc = event.chunk_coord;

            if (chunk_meshes.contains(cc))
            {
                to_remesh.insert(cc);
                // TODO! add adjacent
                // if ()
            }
        }

        for (const auto& cc : to_remesh)
        {
            this->chunk_meshes[cc]->mesh();
        }
    });
}

WorldRenderer::~WorldRenderer()
{
    LOG_INFO("???");
    block_textures_destroy(&this->block_textures);
}
