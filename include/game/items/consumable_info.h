#pragma once

#include <gdfe/../../../gdfe/include/pub/core.h>

typedef struct GDF_ConsumableInfo {
    f32 hp_gain;
    GDF_BOOL ignores_hp_limit;
    f32 hunger_gain;
} GDF_ConsumableInfo;