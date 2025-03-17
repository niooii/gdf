#include <graphics/renderer.h>

WorldRenderer::WorldRenderer(const GDF_VkRenderContext* vk_ctx)
{
    block_textures_init(vk_ctx, &this->block_textures);
    terrain_pipeline_init(vk_ctx, this);
}

WorldRenderer::~WorldRenderer()
{
    LOG_INFO("???");
    block_textures_destroy(&this->block_textures);
}
