#pragma once

#include <../../gdfe/include/core.h>
#include <vulkan/vulkan.h>

#define MAX_TEXTURES = 2048

typedef enum GDF_TEXTURE_INDEX {
    GDF_TEXTURE_INDEX_DIRT,
    GDF_TEXTURE_INDEX_GRASS_TOP,
    GDF_TEXTURE_INDEX_GRASS_SIDE,
    GDF_TEXTURE_INDEX_STONE,
    GDF_TEXTURE_INDEX_MAX,
} GDF_TEXTURE_INDEX;

// This initializes the texture array buffer
// and copies block_texture_ids[] from block.c to a storage buffer.
// bool block_textures_init(VkRenderContext* context, block_textures* out_texture_manager);
//
// bool block_textures_destroy(block_textures* texture_manager);