#include <graphics/chunkmesh.h>
#include <gdfe/render/vk_utils.h>

#include "gdfe/collections/hashmap.h"
#include "graphics/renderer.h"

// #define GDFP_DISABLE

static const VkVertexInputAttributeDescription vertex_attrs[] = {
    {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R8_UINT,
        .offset = 0
    },
    {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R8_UINT,
        .offset = 1
    },
    {
        .binding = 0,
        .location = 2,
        .format = VK_FORMAT_R8_UINT,
        .offset = 2
    },
    {
        .binding = 0,
        .location = 3,
        .format = VK_FORMAT_R8_UINT,
        .offset = 3
    },
    {
        .binding = 0,
        .location = 4,
        .format = VK_FORMAT_R16_UINT,
        .offset = 4
    },
    {
        .binding = 0,
        .location = 5,
        .format = VK_FORMAT_R8_UINT,
        .offset = 6
    },
    {
        .binding = 0,
        .location = 6,
        .format = VK_FORMAT_R8_UINT,
        .offset = 7
    },
};

void get_vertex_attrs(VkVertexInputAttributeDescription** attrs, u32* len)
{
    *attrs = const_cast<VkVertexInputAttributeDescription*>(vertex_attrs);
    *len = std::size(vertex_attrs);
}

void print_mask(const char* msg, u64 n)
{
    char s[65]; 
    GDF_MemSet(s, 0, sizeof(s));
    for (u64 i = 1ULL << 63; i > 0; i = i / 2) {
        if ((n & i) != 0) {
            strcat(s, "1");
        }
        else {
            strcat(s, "0");
        }
    }
    LOG_DEBUG("%s%s", msg, s);
}

void print_mask_64(const char* msg, u64 n)
{
    char s[65]; 
    GDF_MemSet(s, 0, sizeof(s));
    for (u64 i = 1ULL << 63; i > 0; i = i / 2) {
        if ((n & i) != 0) {
            strcat(s, "1");
        }
        else {
            strcat(s, "0");
        }
    }
    LOG_DEBUG("%s%s", msg, s);
}

void print_mask_32(const char* msg, u32 n)
{
    char s[33];
    GDF_MemSet(s, 0, sizeof(s));
    for (u32 i = 1U << 31; i > 0; i = i / 2) {
        if ((n & i) != 0) {
            strcat(s, "1");
        }
        else {
            strcat(s, "0");
        }
    }
    LOG_DEBUG("%s%s", msg, s);
}

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

#define AXIS_X_ASC 0
#define AXIS_X_DESC 1
#define AXIS_Y_ASC 2
#define AXIS_Y_DESC 3
#define AXIS_Z_ASC 4
#define AXIS_Z_DESC 5

FORCEINLINE BLOCK_FACE __axis_to_face(u32 axis)
{
    switch (axis) {
        case AXIS_X_ASC:
            return BLOCK_FACE_RIGHT;
        case AXIS_X_DESC:
            return BLOCK_FACE_LEFT;
        case AXIS_Y_ASC:
            return BLOCK_FACE_TOP;
        case AXIS_Y_DESC:
            return BLOCK_FACE_BOT;
        case AXIS_Z_ASC:
            return BLOCK_FACE_FRONT;
        case AXIS_Z_DESC:
            return BLOCK_FACE_BACK;
    }
    LOG_FATAL("are u stupid");
    return BLOCK_FACE_BOT;
}

FORCEINLINE ivec3 __get_actual_coord(u32 axis_depth, BLOCK_FACE face, u32 rel_x, u32 rel_y) 
{
    switch (face) {
        case BLOCK_FACE_TOP:
            return ivec3_new(rel_x, axis_depth + 1, rel_y);
        case BLOCK_FACE_BOT:
            return ivec3_new(rel_x, axis_depth, rel_y);
        case BLOCK_FACE_LEFT:
            return ivec3_new(axis_depth, rel_y, rel_x);
        case BLOCK_FACE_RIGHT:
            return ivec3_new(axis_depth + 1, rel_y, rel_x);
        case BLOCK_FACE_FRONT:
            return ivec3_new(rel_x, rel_y, axis_depth + 1);
        case BLOCK_FACE_BACK:
            return ivec3_new(rel_x, rel_y, axis_depth);
    }
    LOG_FATAL("are u stupid 2");
    return ivec3_zero();
}

// Implementation of a binary greedy meshing algorithm.
// WARNING: this is a very specific impl that only works for 32x32x32. take a look later..
void ChunkMesh::mesh()
{
    #include <gdfe/profiler.h>

    World* world = world_;
    Chunk* chunk = chunk_;
    vertices_.clear();
    indices_.clear();

    // stores the bitmasks for each axis x y and z
    u64 axis_masks[3][CHUNK_SIZE_P][CHUNK_SIZE_P];
    // stores the masks for culled faces, where 1 represents a visible face and 0 doesnt.  
    u64 axis_face_masks[6][CHUNK_SIZE_P][CHUNK_SIZE_P];

    GDF_MemSet(axis_masks, 0, sizeof(axis_masks));
    GDF_MemSet(axis_face_masks, 0, sizeof(axis_face_masks));

    GDFP_START();

    // TODO! MAIN BOTTLENECK HELP
    for (i32 x = 0; x < CHUNK_SIZE_P; x++) 
    {
        for (i32 y = 0; y < CHUNK_SIZE_P; y++) 
        {
            for (i32 z = 0; z < CHUNK_SIZE_P; z++) 
            {
                vec3 world_pos = vec3_add(chunk_coord_to_world_pos(chunk_coord_), vec3_new(x-1, y-1, z-1));
                
                // TODO! could replace this entire loop with just iterating through the chunks
                // owned blocks, then 8 other for loops for the outer edges of the chunk (the neighboring stuff)
                // TODO! filter blocks by visibility and solidness not existence
                if (world->get_block(world_pos)) {
                    // z,y - x axis
                    axis_masks[AXIS_X][y][z] |= 1ULL << (u64)x;
                    // x,z - y axis
                    axis_masks[AXIS_Y][z][x] |= 1ULL << (u64)y;
                    // x,y - z axis
                    axis_masks[AXIS_Z][y][x] |= 1ULL << (u64)z;
                }
            }
        }
    }
    GDFP_LOG_MSG_RESET("Finished initializing axis masks with chunk data.");

    for (u32 axis = 0; axis < 3; axis++)
    {
        for (u32 i = 0; i < CHUNK_SIZE_P; i++)
        {
            for (u32 j = 0; j < CHUNK_SIZE_P; j++)
            {
                u64 bits = axis_masks[axis][i][j];

                // TODO! does this work like this
                if (!bits)
                    continue;  
                    
                // ascending (forward) axis
                axis_face_masks[2 * axis][i][j] = (bits & ~(bits >> 1));
                // descending (backward) axis
                axis_face_masks[2 * axis + 1][i][j] = (bits & ~(bits << 1));
            
                // print_mask("asc axis: ", axis_face_masks[2 * axis][i][j]);
                // print_mask("desc axis: ", axis_face_masks[2 * axis + 1][i][j]);
            }
        }
    }
    GDFP_LOG_MSG_RESET("Finished face culling.");

    using ankerl::unordered_dense::map, std::array;
    // One map per axis.
    // keys for the outer map are just the block type.
    // each outer map is a hashmap of the pair:
    // <block_type: u32, GDF_HashMap(axis_depth: u32, plane: u32[32])>
    // each inner map is of the pair:
    // <axis_depth: u32, plane: u32[32] (will be manually allocated when needed)>
    // TODO! might as well just use a flat array oml
    map<u32, map<u32, array<u32, 32>>> planes[6] = {};

    for (u32 axis = 0; axis < 6; axis++) 
    {
        for (u32 i = 0; i < CHUNK_SIZE; i++)
        {
            for (u32 j = 0; j < CHUNK_SIZE; j++)
            {
                // skip neighboring chunk masks
                u64 bits = axis_face_masks[axis][i + 1][j + 1];

                // remove the rightmost and leftmost bit, we dont care about those
                bits >>= 1;
                bits &= ~(1ULL << CHUNK_SIZE);

                while (bits != 0)
                {
                    // where the face is (kinda)
                    // more like how deep it is along the axis
                    u8 depth = CTZ64(bits);
                    bits &= bits - 1;

                    RelBlockCoord block_coord;
                    switch(axis) {
                    case AXIS_X_ASC:
                    case AXIS_X_DESC:
                        block_coord.block_x = depth;
                        block_coord.block_y = i;
                        block_coord.block_z = j;
                        break;
                    case AXIS_Y_ASC:
                    case AXIS_Y_DESC:
                        block_coord.block_x = j;
                        block_coord.block_y = depth;
                        block_coord.block_z = i;
                        break;
                    case AXIS_Z_ASC:
                    case AXIS_Z_DESC:
                        block_coord.block_x = j;
                        block_coord.block_y = i;
                        block_coord.block_z = depth;
                        break;
                    }

                    Block* block = chunk->get_block(block_coord);

                    if (!block) {
                        LOG_FATAL("yea ur code is so bad");
                        continue;
                    }

                    map<u32, array<u32, 32>>& depth_map =
                        planes[axis].try_emplace(block->data.type).first->second;
                    depth_map[depth] = depth_map.try_emplace(depth).first->second;

                    array<u32, 32>& plane = depth_map[depth];
                    plane[j] |= (u32)1 << i;
                }
            }
        }
    }
    GDFP_LOG_MSG_RESET("Finished generating binary planes.");

    // iterate for each axis (forward and backward), then for each block type in the map we get all the planes that
    // need to be meshed at a specific depth. 
    for (u32 axis = 0; axis < 6; axis++) {
        BLOCK_FACE face = __axis_to_face(axis);
        for (auto& [block_type, depth_map] : planes[axis]) {
            // no type chekcing is so scary help m,e
            // update: c++...
            for (auto& [depth, plane] : depth_map) {
                // actual greedy meshing algorithim
                // debugging stuff
                // for (int k = 0; k < 32; k++) {
                //     print_mask_32("mask: ", plane[k]);
                // }
                for (u32 row = 0; row < CHUNK_SIZE; row++) {
                    // x is the current x position within this row
                    u32 x = 0;
                    while (x < CHUNK_SIZE) {
                        // TODO! could we pregenerate this per chunk and then just update a few bits
                        // on each chunk change, then deep copy the data in this function?
                        // im p sure itll be faster
                        x += CTZ64(plane[row] >> x);
                        if (x >= CHUNK_SIZE)
                            continue;

                        u32 w = CTZ64(~(plane[row] >> x));
                        // print_mask_32("mask: ", plane[row]);
                        // LOG_DEBUG("width: %d", w);
                        // account for overflow (where shifting is undefined SCARY!!)
                        u32 w_mask = ((w >= 32) ? 0xFFFFFFFF : ((1ULL << w) - 1)) << x;

                        u32 h = 1;
                        // if you ever change chunk size beware of this
                        while (row + h < CHUNK_SIZE) {
                            if ((w_mask & plane[row + h]) != w_mask)
                                break;

                            // set all the processed bits to 0
                            plane[row + h] &= ~w_mask;

                            h++;
                        }

                        // push the vertices for the quad we just made
                        // LOG_DEBUG("depth: %d", depth);
                        ivec3 v0_pos = __get_actual_coord(depth, face, row, x);
                        ivec3 v1_pos = __get_actual_coord(depth, face, row + h, x);
                        ivec3 v2_pos = __get_actual_coord(depth, face, row + h, x + w);
                        ivec3 v3_pos = __get_actual_coord(depth, face, row, x + w);
                        u32 num_vertices = vertices_.size();
                        vertices_.push_back((ChunkVertex){
                            .block_type = (u16)block_type,
                            .face_dir = (u8)face,
                            .x_pos = (u8)v0_pos.x,
                            .y_pos = (u8)v0_pos.y,
                            .z_pos = (u8)v0_pos.z,
                            .u = 0,
                            .v = 0
                        });
                        vertices_.push_back((ChunkVertex){
                            .block_type = (u16)block_type,
                            .face_dir = (u8)face,
                            .x_pos = (u8)v1_pos.x,
                            .y_pos = (u8)v1_pos.y,
                            .z_pos = (u8)v1_pos.z,
                            .u = (u8)h,
                            .v = 0
                        });
                        vertices_.push_back((ChunkVertex){
                            .block_type = (u16)block_type,
                            .face_dir = (u8)face,
                            .x_pos = (u8)v2_pos.x,
                            .y_pos = (u8)v2_pos.y,
                            .z_pos = (u8)v2_pos.z,
                            .u = (u8)h,
                            .v = (u8)w
                        });
                        vertices_.push_back((ChunkVertex){
                            .block_type = (u16)block_type,
                            .face_dir = (u8)face,
                            .x_pos = (u8)v3_pos.x,
                            .y_pos = (u8)v3_pos.y,
                            .z_pos = (u8)v3_pos.z,
                            .u = 0,
                            .v = (u8)w
                        });

                        // push indices based on face dir for consistent winding
                        switch (face) {
                            case BLOCK_FACE_FRONT: // +Z face
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 3); // v3
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices + 1); // v1
                                break;

                            case BLOCK_FACE_BACK: // -Z face
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 1); // v1
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices + 3); // v3
                                break;

                            case BLOCK_FACE_RIGHT: // +X face
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 1); // v1
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices + 3); // v3
                                break;

                            case BLOCK_FACE_LEFT: // -X face
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 3); // v3
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices + 1); // v1
                                break;

                            case BLOCK_FACE_TOP: // +Y face
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 1); // v1
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices + 3); // v3
                                break;

                            case BLOCK_FACE_BOT: // -Y face
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 3); // v3
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices);     // v0
                                indices_.push_back(num_vertices + 2); // v2
                                indices_.push_back(num_vertices + 1); // v1
                                break;
                        }
                        x += w;
                    }
                }
            }
        }
    }

    GDFP_LOG_MSG_RESET("Finished meshing algorithm.");

    // finalization
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        buffers_[i].up_to_date = false;
    }
    GDFP_END();
}

u32 ChunkMesh::get_index_count()
{
    return indices_.size();
}

ChunkMesh::ChunkMesh(World* world, Chunk* chunk, ivec3 chunk_coord)
    : chunk_(chunk), world_(world), chunk_coord_(chunk_coord)
{
    vertices_.reserve(256);
    indices_.reserve(256);

    this->mesh();

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (!GDF_VkBufferCreateVertex(
            NULL,
            MAX_CHUNK_VERTICES,
            sizeof(ChunkVertex),
            &buffers_[i].vertex_buffer
        ))
        {
            LOG_FATAL("buffer cooked");
        }
        if (!GDF_VkBufferCreateIndex(
            NULL,
            MAX_CHUNK_INDICES,
            &buffers_[i].index_buffer
        ))
        {
            LOG_FATAL("buffer cooked");
        }
        // update info for all buffers in the init pass as well.
        if (!this->update_buffers(i)) {
            LOG_FATAL("Failed to update chunkmesh buffer");
        }
    }
}

ChunkMesh::~ChunkMesh()
{
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        GDF_VkBufferDestroy(
            &buffers_[i].vertex_buffer
        );
        GDF_VkBufferDestroy(
            &buffers_[i].index_buffer
        );
    }
}

bool ChunkMesh::update_buffers(u32 frame_idx)
{
    if (buffers_[frame_idx].up_to_date)
        return true;
    if (!GDF_VkBufferUpdate(
        &buffers_[frame_idx].vertex_buffer,
        vertices_.data(),
        vertices_.size() * sizeof(ChunkVertex)
    ))
        return false;

    if (!GDF_VkBufferUpdate(
        &buffers_[frame_idx].index_buffer,
        indices_.data(),
        indices_.size() * sizeof(CHUNK_MESH_INDEX_TYPE)
    ))
        return false;

    buffers_[frame_idx].up_to_date = true;
    return true;
}
