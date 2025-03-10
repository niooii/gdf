#pragma once

#include <../../../gdfe/include/pub/core.h>

typedef struct GDF_WeaponInfo {
    f32 damage;
    f32 swing_speed;
    bool blockable;
    bool parryable;
} GDF_WeaponInfo;