#include <client/graphics/pipelines.h>
#include <gdfe/render/vk/utils.h>
#include <gdfe/render/vk/types.h>
#include <client/graphics/chunkmesh.h>
#include <client/graphics/renderer.h>
#include <gdfe/render/vk/buffers.h>

bool terrain_pipeline_init(const GDF_VkRenderContext* vk_ctx, WorldRenderer* world_renderer)
{
    terrain_pipeline* pipeline = &world_renderer->terrain_pipeline;
    // Copy all static block data to a storage buffer
    GDF_VkBufferCreateStorage(
        (void*)STATIC_BLOCK_LOOKUP_TABLE,
        STATIC_BLOCK_LOOKUP_TABLE_SIZE,
        &pipeline->block_lookup_ssbo
    );
    VkDescriptorSetLayoutBinding layout_bindings[] = {
        {
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        },
        {
            .binding = 1,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        }
    };

    VkDescriptorSetLayoutCreateInfo layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = sizeof(layout_bindings)/sizeof(VkDescriptorSetLayoutBinding),
        .pBindings = layout_bindings,
    };

    u32 image_count = vk_ctx->swapchain.image_count;

    VK_ASSERT(
        vkCreateDescriptorSetLayout(
            vk_ctx->device.handle,
            &layout_create_info,
            vk_ctx->device.allocator,
            &pipeline->descriptor_layout
        )
    );

    pipeline->descriptor_sets.resize(image_count);

    VkDescriptorPoolSize pool_size = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = image_count
    };
    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
        .maxSets = image_count
    };

    VK_ASSERT(
        vkCreateDescriptorPool(
            vk_ctx->device.handle,
            &pool_info,
            vk_ctx->device.allocator,
            &pipeline->descriptor_pool
        )
    );

    VkDescriptorSetAllocateInfo set_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pipeline->descriptor_pool,
        .pSetLayouts = &pipeline->descriptor_layout,
        .descriptorSetCount = 1
    };

    for (int i = 0; i < image_count; i++)
    {
        // Allocate sets
        VK_ASSERT(
            vkAllocateDescriptorSets(
                vk_ctx->device.handle,
                &set_alloc_info,
                &pipeline->descriptor_sets[i]
            )
        );
    }

    // Update sets
    // Fragment texture sampler
    VkDescriptorImageInfo image_info = {
        .sampler = world_renderer->block_textures.sampler,
        .imageView = world_renderer->block_textures.texture_array.view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    // SSBO for static block data for gpu lookups
    VkDeviceSize offset = {0};
    VkDescriptorBufferInfo buffer_info = {
        .buffer = pipeline->block_lookup_ssbo.handle,
        .offset = offset,
        .range = VK_WHOLE_SIZE
    };

    for (u32 i = 0; i < image_count; i++)
    {
        VkWriteDescriptorSet descriptor_writes[] = {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = pipeline->descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .pImageInfo = &image_info
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = pipeline->descriptor_sets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &buffer_info
            }
        };
        vkUpdateDescriptorSets(
            vk_ctx->device.handle,
            sizeof(descriptor_writes) / sizeof(VkWriteDescriptorSet),
            descriptor_writes,
            0,
            NULL
        );
    }

    // Vertex input configuration
    VkVertexInputBindingDescription bindings = {
        .binding = 0,
        .stride = sizeof(ChunkVertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription* attributes;
    u32 attr_len;

    get_vertex_attrs(&attributes, &attr_len);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pVertexBindingDescriptions = &bindings,
        .vertexBindingDescriptionCount = 1,
        .pVertexAttributeDescriptions = attributes,
        .vertexAttributeDescriptionCount = attr_len,
    };

    // Input assembly configuration
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .primitiveRestartEnable = VK_FALSE,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    // Viewport and scissor configuration (dynamic states)
    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    // Rasterization configuration
    VkPipelineRasterizationStateCreateInfo rasterizer_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        // .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE
    };

    // Multisampling configuration
    VkPipelineMultisampleStateCreateInfo multisampling_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = vk_ctx->msaa_samples,
    };

    // Depths stencil configuration
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };

    // Color blending configuration
    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
    };

    VkDescriptorSetLayout descriptor_layouts[2] = {
        vk_ctx->vp_ubo_layout,
        pipeline->descriptor_layout
    };

    VkPushConstantRange push_constant_ranges[] = {
        {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            // the chunk coordinate as a push constant.
            .size = sizeof(ivec3)
        },
    };

    // Pipeline layout
    VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pSetLayouts = descriptor_layouts,
        .setLayoutCount = sizeof(descriptor_layouts) / sizeof(VkDescriptorSetLayout),
        .pPushConstantRanges = push_constant_ranges,
        .pushConstantRangeCount = sizeof(push_constant_ranges) / sizeof(VkPushConstantRange)
    };

    VK_ASSERT(
        vkCreatePipelineLayout(
            vk_ctx->device.handle,
            &layout_info,
            vk_ctx->device.allocator,
            &pipeline->layout
        )
    );

    // Create wireframe layout
    VK_ASSERT(
        vkCreatePipelineLayout(
            vk_ctx->device.handle,
            &layout_info,
            vk_ctx->device.allocator,
            &pipeline->wireframe_layout
        )
    );

    // Create dynamic state for pipeline (viewport & scissor)
    // TODO! eventually if the game is fixed size remove these and bake states
    // into pipelines
    VkDynamicState d_states[2] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamic_states = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = d_states
    };

    pipeline->vert = GDF_VkUtilsLoadShader("resources/shaders/blocks.vert.spv");
    pipeline->frag = GDF_VkUtilsLoadShader("resources/shaders/blocks.frag.spv");

    VkPipelineShaderStageCreateInfo block_shaders[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = pipeline->vert,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = pipeline->frag,
            .pName = "main"
        }
    };

    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = sizeof(block_shaders) / sizeof(VkPipelineShaderStageCreateInfo),
        .pStages = block_shaders,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer_state,
        .pMultisampleState = &multisampling_state,
        .pDepthStencilState = &depth_stencil_state,
        .pColorBlendState = &color_blend_state,
        .layout = pipeline->layout,
        .pDynamicState = &dynamic_states,
    };

    GDF_VkPipelineInfoFillGeometryPass(&pipeline_create_info);

    VK_ASSERT(
        vkCreateGraphicsPipelines(
            vk_ctx->device.handle,
            VK_NULL_HANDLE,
            1,
            &pipeline_create_info,
            vk_ctx->device.allocator,
            &pipeline->handle
        )
    );
    pipeline_create_info.layout = pipeline->wireframe_layout;
    rasterizer_state.polygonMode = VK_POLYGON_MODE_LINE;
    VK_ASSERT(
        vkCreateGraphicsPipelines(
            vk_ctx->device.handle,
            VK_NULL_HANDLE,
            1,
            &pipeline_create_info,
            vk_ctx->device.allocator,
            &pipeline->wireframe_handle
        )
    );

    return GDF_TRUE;
}

// GDF_BOOL pipelines_create_ui(VkRenderContext* context)
// {
//     pipeline_ui* pipeline = &context->ui_pipeline;
//
//      // Vertex input configuration
//     VkVertexInputBindingDescription bindings = {
//         .binding = 0,
//         .stride = sizeof(Vertex3d),
//         .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
//     };
//
//     VkVertexInputAttributeDescription attributes[] = {
//         {
//             .binding = 0,
//             .location = 0,
//             .format = VK_FORMAT_R32G32B32_SFLOAT,
//             .offset = offsetof(Vertex3d, pos)
//         },
//     };
//
//     VkPipelineVertexInputStateCreateInfo vertex_input_info = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
//         .vertexBindingDescriptionCount = 1,
//         .pVertexBindingDescriptions = &bindings,
//         .vertexAttributeDescriptionCount = sizeof(attributes) / sizeof(attributes[0]),
//         .pVertexAttributeDescriptions = attributes
//     };
//
//     // Input assembly configuration
//     VkPipelineInputAssemblyStateCreateInfo input_assembly = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
//         .primitiveRestartEnable = VK_FALSE,
//         .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
//     };
//
//     // Viewport and scissor configuration (dynamic states)
//     VkPipelineViewportStateCreateInfo viewport_state = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
//         .viewportCount = 1,
//         .scissorCount = 1,
//     };
//
//     // Rasterization configuration
//     VkPipelineRasterizationStateCreateInfo rasterizer_state = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//         .depthClampEnable = VK_FALSE,
//         .rasterizerDiscardEnable = VK_FALSE,
//         .polygonMode = VK_POLYGON_MODE_FILL,
//         .lineWidth = 1.0f,
//         .cullMode = VK_CULL_MODE_NONE,
//         .frontFace = VK_FRONT_FACE_CLOCKWISE,
//         .depthBiasEnable = VK_FALSE
//     };
//
//     // Multisampling configuration
//     VkPipelineMultisampleStateCreateInfo multisampling_state = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
//         .sampleShadingEnable = VK_FALSE,
//         .rasterizationSamples = context->msaa_samples,
//     };
//
//     // Depths stencil configuration
//     VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
//         .depthTestEnable = VK_TRUE,
//         .depthWriteEnable = VK_TRUE,
//         .depthCompareOp = VK_COMPARE_OP_LESS,
//         .depthBoundsTestEnable = VK_FALSE,
//         .stencilTestEnable = VK_FALSE
//     };
//
//     // Color blending configuration
//     VkPipelineColorBlendAttachmentState color_blend_attachment = {
//         .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
//         .blendEnable = VK_TRUE,
//         .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
//         .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
//         .colorBlendOp = VK_BLEND_OP_ADD,
//         .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
//         .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
//         .alphaBlendOp = VK_BLEND_OP_ADD
//     };
//
//     VkPipelineColorBlendStateCreateInfo color_blend_state = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
//         .logicOpEnable = VK_FALSE,
//         .attachmentCount = 1,
//         .pAttachments = &color_blend_attachment,
//         .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
//     };
//
//     VkPushConstantRange push_constant_range = {
//         .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
//         .offset = 0,
//         .size = sizeof(vec3)
//     };
//
//     // Pipeline layout
//     // TODO! dont need uniform matrices
//     VkPipelineLayoutCreateInfo layout_info = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
//         .pSetLayouts = &context->global_vp_ubo_layouts[0],
//         .setLayoutCount = 1,
//         .pPushConstantRanges = &push_constant_range,
//         .pushConstantRangeCount = 1
//     };
//
//     VK_ASSERT(
//         vkCreatePipelineLayout(
//             context->device.handle,
//             &layout_info,
//             context->device.allocator,
//             &context->grid_pipeline.layout
//         )
//     );
//
//     // Create dynamic state for pipeline (viewport & scissor)
//     // TODO! eventually if the game is fixed size remove these and bake states
//     // into pipelines
//     VkDynamicState d_states[2] = {
//         VK_DYNAMIC_STATE_VIEWPORT,
//         VK_DYNAMIC_STATE_SCISSOR
//     };
//     VkPipelineDynamicStateCreateInfo dynamic_states = {
//         .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
//         .dynamicStateCount = 2,
//         .pDynamicStates = d_states
//     };
//
//     pipeline->vert = context->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_UI_VERT];
//     pipeline->frag = context->builtin_shaders[GDF_VK_SHADER_MODULE_INDEX_UI_FRAG];
//
//     VkPipelineShaderStageCreateInfo ui_shaders[] = {
//         {
//             .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//             .stage = VK_SHADER_STAGE_VERTEX_BIT,
//             .module = pipeline->vert,
//             .pName = "main"
//         },
//         {
//             .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//             .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
//             .module = pipeline->frag,
//             .pName = "main"
//         }
//     };
//
//     return GDF_TRUE;
// }