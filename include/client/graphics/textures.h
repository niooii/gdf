#pragma once

#include <gdfe/prelude.h>
#include <vulkan/vulkan.h>

#include "gdfe/render/vk/types.h"

#define MAX_TEXTURES = 2048

typedef enum GDF_TEXTURE_INDEX {
    GDF_TEXTURE_INDEX_DIRT,
    GDF_TEXTURE_INDEX_GRASS_TOP,
    GDF_TEXTURE_INDEX_GRASS_SIDE,
    GDF_TEXTURE_INDEX_STONE,
    GDF_TEXTURE_INDEX_WOOD_PLANK,
    GDF_TEXTURE_INDEX_MAX,
} GDF_TEXTURE_INDEX;

typedef struct block_textures {
    const GDF_VkDevice*    device;
    VkAllocationCallbacks* allocator;

    GDF_VkImage    texture_array;
    VkDeviceMemory texture_array_memory;
    VkSampler      sampler;
} block_textures;

// This initializes the texture array buffer
// and copies block_texture_ids[] from block.c to a storage buffer.
bool block_textures_init(const GDF_VkRenderContext* vk_ctx, block_textures* out);

void block_textures_destroy(block_textures* textures);
