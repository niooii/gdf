#pragma once

#include <vector>
#include <gdfe/core.h>
#include <gdfe/render/vk/types.h>

#include "textures.h"

typedef struct terrain_pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;

    VkPipeline wireframe_handle;
    VkPipelineLayout wireframe_layout;

    VkShaderModule vert;
    VkShaderModule frag;

    // Shader storage buffer, accessed through instancing.
    // TODO! vertex/face pulling
    // GDF_VkBuffer face_data_ssbo;

    GDF_VkBuffer block_lookup_ssbo;
    VkDescriptorPool descriptor_pool;
    std::vector<VkDescriptorSet> descriptor_sets;
    VkDescriptorSetLayout descriptor_layout;
} terrain_pipeline;

typedef struct WorldRenderer WorldRenderer;

bool terrain_pipeline_init(const GDF_VkRenderContext* vk_ctx, WorldRenderer* world_renderer);

