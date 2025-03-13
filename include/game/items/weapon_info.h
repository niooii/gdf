#pragma once

#include <gdfe/../../../gdfe/include/pub/core.h>

typedef struct GDF_WeaponInfo {
    f32 damage;
    f32 swing_speed;
    GDF_BOOL blockable;
    GDF_BOOL parryable;
} GDF_WeaponInfo;