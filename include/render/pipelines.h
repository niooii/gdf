#pragma once

#include <core.h>
#include <render/vk_types.h>

typedef struct block_textures {
    vk_device* device;
    VkAllocationCallbacks* allocator;

    vk_image texture_array;
    VkDeviceMemory texture_array_memory;
    VkSampler sampler;
} block_textures;

typedef struct pipeline_block {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkPipeline wireframe_handle;
    VkPipelineLayout wireframe_layout;

    VkShaderModule vert;
    VkShaderModule frag;

    // Shader storage buffer, accessed through instancing.
    buffer face_data_ssbo;

    block_textures block_textures;
    buffer block_lookup_ssbo;
    VkDescriptorPool descriptor_pool;
    GDF_LIST(VkDescriptorSet) descriptor_sets;
    GDF_LIST(VkDescriptorSetLayout) descriptor_layouts;
} pipeline_block;

bool pipelines_create_blocks(VkRenderContext* context);
// bool pipelines_create_lighting(VkRenderContext* context);
// bool pipelines_create_post_processing(VkRenderContext* context);
bool pipelines_create_grid(VkRenderContext* context);
bool pipelines_create_ui(VkRenderContext* context);