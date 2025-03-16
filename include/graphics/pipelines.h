#pragma once

#include <gdfe/core.h>
#include <gdfe/render/vk_types.h>
#include "textures.h"

typedef struct terrain_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkPipeline wireframe_handle;
    VkPipelineLayout wireframe_layout;

    VkShaderModule vert;
    VkShaderModule frag;

    // Shader storage buffer, accessed through instancing.
    GDF_VkBuffer face_data_ssbo;

    block_textures block_textures;
    GDF_VkBuffer block_lookup_ssbo;
    VkDescriptorPool descriptor_pool;
    GDF_LIST(VkDescriptorSet) descriptor_sets;
    VkDescriptorSetLayout descriptor_layouts;
} terrain_pipeline;
//
// GDF_BOOL pipelines_create_blocks(VkRenderContext* context);
// // GDF_BOOL pipelines_create_lighting(VkRenderContext* context);
// // GDF_BOOL pipelines_create_post_processing(VkRenderContext* context);
// GDF_BOOL pipelines_create_grid(VkRenderContext* context);
// GDF_BOOL pipelines_create_ui(VkRenderContext* context);