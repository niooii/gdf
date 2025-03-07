#pragma once

#include <core.h>
#include <render/vk_types.h>

bool pipelines_create_blocks(VkRenderContext* context);
// bool pipelines_create_lighting(VkRenderContext* context);
// bool pipelines_create_post_processing(VkRenderContext* context);
bool pipelines_create_grid(VkRenderContext* context);
bool pipelines_create_ui(VkRenderContext* context);