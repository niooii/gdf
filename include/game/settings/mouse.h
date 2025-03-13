#pragma once

#include <gdfe/../../../gdfe/include/pub/core.h>

typedef struct GDF_MouseSettings {
    f32 x_sensitivity;
    f32 y_sensitivity;
    f32 max_sensitivity;
    f32 min_sensitivity;
    GDF_BOOL inverted;
} GDF_MouseSettings;