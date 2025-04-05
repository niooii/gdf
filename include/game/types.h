#pragma once
#include <gdfe/def.h>

typedef union u8vec3_u {
    u8 elements[3];
    struct {
        u8 x;
        u8 y;
        u8 z;
    };
} u8vec3;