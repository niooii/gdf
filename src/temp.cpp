/*

GDF_BOOL __create_shader_modules()
{
    // load all (built in) shaders
    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/blocks.vert.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_BLOCKS_VERT]
        )
    )
    {
        LOG_ERR("Failed to create geometry pass vertex shader. exiting...");
        return GDF_FALSE;
    }

    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/blocks.frag.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_BLOCKS_FRAG]
        )
    )
    {
        LOG_ERR("Failed to create geometry pass fragment shader. exiting...");
        return GDF_FALSE;
    }

    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/lighting_base.vert.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_LIGHTING_VERT]
        )
    )
    {
        LOG_ERR("Failed to create lighting pass vertex shader. exiting...");
        return GDF_FALSE;
    }

    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/lighting_base.frag.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_LIGHTING_FRAG]
        )
    )
    {
        LOG_ERR("Failed to create lighting pass fragment shader. exiting...");
        return GDF_FALSE;
    }


    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/grid.vert.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_GRID_VERT]
        )
    )
    {
        LOG_ERR("Failed to create grid fragment shader. exiting...");
        return GDF_FALSE;
    }

    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/grid.frag.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_GRID_FRAG]
        )
    )
    {
        LOG_ERR("Failed to create grid fragment shader. exiting...");
        return GDF_FALSE;
    }

    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/post_processing_base.vert.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_POST_PROCESS_VERT]
        )
    )
    {
        LOG_ERR("Failed to create post processing pass vertex shader. exiting...");
        return GDF_FALSE;
    }

    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/post_processing_base.frag.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_POST_PROCESS_FRAG]
        )
    )
    {
        LOG_ERR("Failed to create post processing pass fragment shader. exiting...");
        return GDF_FALSE;
    }

    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/ui.vert.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_UI_VERT]
        )
    )
    {
        LOG_ERR("Failed to create UI vertex shader. exiting...");
        return GDF_FALSE;
    }

    if (
        !utils_create_shader_module(
            vk_ctx,
            "resources/shaders/ui.frag.spv",
            &vk_ctx->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_UI_FRAG]
        )
    )
    {
        LOG_ERR("Failed to create post processing pass fragment shader. exiting...");
        return GDF_FALSE;
    }

    return GDF_TRUE;
}
*/