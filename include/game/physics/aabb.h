#pragma once

#include <gdfe/core.h>
#include <gdfe/math/math.h>

typedef enum AXIS {
    AXIS_POS_X,
} AXIS;

typedef struct AxisAlignedBoundingBox {
    vec3 min;
    vec3 max;
} AxisAlignedBoundingBox;

// Returns whether an aabb is colliding with another aabb->
FORCEINLINE GDF_BOOL aabb_collides(AxisAlignedBoundingBox* a, AxisAlignedBoundingBox* b) 
{
    return a->min.x <= b->max.x &&
    a->max.x >= b->min.x &&
    a->min.y <= b->max.y &&
    a->max.y >= b->min.y &&
    a->min.z <= b->max.z &&
    a->max.z >= b->min.z;
}

FORCEINLINE GDF_BOOL aabb_intersects(AxisAlignedBoundingBox* a, AxisAlignedBoundingBox* b) 
{
    return a->min.x < b->max.x &&
    a->max.x > b->min.x &&
    a->min.y < b->max.y &&
    a->max.y > b->min.y &&
    a->min.z < b->max.z &&
    a->max.z > b->min.z;
}

FORCEINLINE vec3 aabb_get_center(AxisAlignedBoundingBox* a)
{
    return vec3_new(
        (a->min.x + a->max.x) / 2,
        (a->min.y + a->max.y) / 2,
        (a->min.z + a->max.z) / 2
    );
}

FORCEINLINE vec3 aabb_bot_left(AxisAlignedBoundingBox* a)
{
    return vec3_new(a->min.x, a->min.y, a->min.z);
}

FORCEINLINE vec3 aabb_bot_right(AxisAlignedBoundingBox* a)
{
    return vec3_new(a->max.x, a->min.y, a->min.z);
}

FORCEINLINE vec3 aabb_top_left(AxisAlignedBoundingBox* a)
{
    return vec3_new(a->min.x, a->max.y, a->min.z);
}

FORCEINLINE vec3 aabb_top_right(AxisAlignedBoundingBox* a)
{
    return vec3_new(a->max.x, a->max.y, a->min.z);
}

FORCEINLINE vec3 aabb_bot_left_back(AxisAlignedBoundingBox* a)
{
    return vec3_new(a->min.x, a->min.y, a->max.z);
}

FORCEINLINE vec3 aabb_bot_right_back(AxisAlignedBoundingBox* a)
{
    return vec3_new(a->max.x, a->min.y, a->max.z);
}

FORCEINLINE vec3 aabb_top_left_back(AxisAlignedBoundingBox* a)
{
    return vec3_new(a->min.x, a->max.y, a->max.z);
}

FORCEINLINE vec3 aabb_top_right_back(AxisAlignedBoundingBox* a)
{
    return vec3_new(a->max.x, a->max.y, a->max.z);
}

// Returns the vector needed to translate aabb a to resolve the intersection.
FORCEINLINE vec3 aabb_get_intersection_resolution(AxisAlignedBoundingBox* a, AxisAlignedBoundingBox* b)
{
    vec3 resolution = {0};
    
    float dx1 = b->max.x - a->min.x;  // overlap when a is to the left
    float dx2 = b->min.x - a->max.x;  // overlap when a is to the right
    float dy1 = b->max.y - a->min.y;  // overlap when a is below
    float dy2 = b->min.y - a->max.y;  // overlap when a is above
    float dz1 = b->max.z - a->min.z;  // overlap when a is in front
    float dz2 = b->min.z - a->max.z;  // overlap when a is behind
    
    // choose the smallest penetration for each axis
    resolution.x = gabs(dx1) < gabs(dx2) ? dx1 : dx2;
    resolution.y = gabs(dy1) < gabs(dy2) ? dy1 : dy2;
    resolution.z = gabs(dz1) < gabs(dz2) ? dz1 : dz2;
    
    // find which axis has smallest penetration
    float absX = gabs(resolution.x);
    float absY = gabs(resolution.y);
    float absZ = gabs(resolution.z);
    
    // only keep smallest penetration axis
    if (absY <= absX && absY <= absZ) 
    {
        resolution.x = 0;
        resolution.z = 0;
    } 
    else if (absX <= absY && absX <= absZ) 
    {
        resolution.y = 0;
        resolution.z = 0;
    } 
    else 
    {
        resolution.x = 0;
        resolution.y = 0;
    }
    
    return resolution;
}

// Translates an aabb by the given translation vector.
FORCEINLINE void aabb_translate(AxisAlignedBoundingBox* a, vec3 t)
{
    vec3_add_to(&a->min, t);
    vec3_add_to(&a->max, t);
}