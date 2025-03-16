#include <physics/raycast.h>
#include <game/world.h>
#include <gdfe/profiler.h>

// LORD save me
// https://gamedev.stackexchange.com/questions/47362/cast-ray-to-select-block-in-voxel-game

// https://github.com/cgyurgyik/fast-voxel-traversal-algorithm/blob/master/overview/FastVoxelTraversalOverview.md
void raycast_blocks(RaycastInfo* info, RaycastBlockHitInfo* result)
{
    i64 block_x = FLOOR(info->origin.x);
    i64 block_y = FLOOR(info->origin.y);
    i64 block_z = FLOOR(info->origin.z);

    i8 step_x = (info->dir.x > 0) ? 1 : ((info->dir.x < 0) ? -1 : 0);
    i8 step_y = (info->dir.y > 0) ? 1 : ((info->dir.y < 0) ? -1 : 0);
    i8 step_z = (info->dir.z > 0) ? 1 : ((info->dir.z < 0) ? -1 : 0);

    BLOCK_FACE x_int = (step_x > 0) ? BLOCK_FACE_LEFT : BLOCK_FACE_RIGHT;
    BLOCK_FACE y_int = (step_y > 0) ? BLOCK_FACE_BOT : BLOCK_FACE_TOP;
    BLOCK_FACE z_int = (step_z > 0) ? BLOCK_FACE_BACK : BLOCK_FACE_FRONT;

    // How far we must move to cross the block boundary
    f32 dx = (info->dir.x != 0) ? gabs(1.0f / info->dir.x) : INFINITY;
    f32 dy = (info->dir.y != 0) ? gabs(1.0f / info->dir.y) : INFINITY;
    f32 dz = (info->dir.z != 0) ? gabs(1.0f / info->dir.z) : INFINITY;

    f32 tx = (info->dir.x != 0) ? 
        ((step_x > 0 ? (block_x + 1 - info->origin.x) : (info->origin.x - block_x)) / gabs(info->dir.x)) : 
        INFINITY;
    f32 ty = (info->dir.y != 0) ? 
        ((step_y > 0 ? (block_y + 1 - info->origin.y) : (info->origin.y - block_y)) / gabs(info->dir.y)) : 
        INFINITY;
    f32 tz = (info->dir.z != 0) ? 
        ((step_z > 0 ? (block_z + 1 - info->origin.z) : (info->origin.z - block_z)) / gabs(info->dir.z)) : 
        INFINITY;

    f32 total_dist = 0.0f;
    BLOCK_FACE last_intersection;
    const int MAX_ITERATIONS = 1000;
    int iterations = 0;

    while (total_dist <= info->max_distance && iterations++ < MAX_ITERATIONS)
    {
        Block* b = info->world->get_block(vec3_new(block_x, block_y, block_z));
        if (b != NULL)
        {
            ivec3 cc = world_pos_to_chunk_coord(vec3_new(block_x, block_y, block_z));
            result->status = RAYCAST_STATUS_HIT;
            result->hit_face = last_intersection;
            result->block = b;
            result->chunk_coord = cc;
            result->block_world_pos = vec3_new(block_x, block_y, block_z);
            result->chunk = info->world->get_or_create_chunk(cc);
            return;
        }

        if (tx <= ty && tx <= tz)
        {
            total_dist = tx;
            block_x += step_x;
            tx += dx;
            last_intersection = x_int;
        }
        else if (ty <= tz)
        {
            total_dist = ty;
            block_y += step_y;
            ty += dy;
            last_intersection = y_int;
        }
        else
        {
            total_dist = tz;
            block_z += step_z;
            tz += dz;
            last_intersection = z_int;
        }
    }

    result->status = RAYCAST_STATUS_ERR_NO_HIT;
}

RaycastEntityHitInfo raycast_entity(RaycastInfo* info)
{
    TODO("entity raycasting");
    // RaycastResult result = {0};

    // vec3 ray_pos = info->origin;
    // if (vec3_length_squared(info->dir) == 0)
    // {
    //     result.status = RAYCAST_STATUS_ERR_NO_DIRECTION;
    //     return result;
    // }
    // vec3 dir = vec3_normalized(info->dir);
    // vec3 step_vec = vec3_mul_scalar(dir, info->step_distance);

    // for (
    //     f32 dist_traveled = 0; 
    //     dist_traveled < info->max_distance; 
    //     dist_traveled += info->step_distance
    // )
    // {
    //     if (info->mode == RAYCAST_MODE_ENTITIES || info->mode == RAYCAST_MODE_WHATEVER)
    //     {
            
    //     }
    //     if (info->mode == RAYCAST_MODE_BLOCKS || info->mode == RAYCAST_MODE_WHATEVER)
    //     {
    //         RaycastBlockHitInfo ret_info = {0};
    //         if (__check_blocks(info->world, ray_pos, &info))
    //         {
    //             result.block_info = ret_info;
    //             result.status = RAYCAST_STATUS_HIT_BLOCK;
    //             return result;
    //         }
    //     }
    //     vec3_add_to(&ray_pos, step_vec);
    // }

    // // one last case at max_distance
    // ray_pos = vec3_add(info->origin, vec3_mul_scalar(dir, info->max_distance));
    // if (info->mode == RAYCAST_MODE_ENTITIES || info->mode == RAYCAST_MODE_WHATEVER)
    // {
    //     TODO("Entity raycasting");
    // }
    // if (info->mode == RAYCAST_MODE_BLOCKS || info->mode == RAYCAST_MODE_WHATEVER)
    // {
    //     RaycastBlockHitInfo ret_info = {0};
    //     if (__check_blocks(info->world, ray_pos, &info))
    //     {
    //         result.block_info = ret_info;
    //         result.status = RAYCAST_STATUS_HIT_BLOCK;
    //         return result;
    //     }
    // }

    // result.status = RAYCAST_STATUS_ERR_NO_HIT;
    // return result;
}
