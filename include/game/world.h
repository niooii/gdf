#pragma once

#include <gdfe/core.h>
#include <game/entity/humanoid.h>
#include <game/physics/physics.h>
#include <game/physics/aabb.h>
#include <gdf_math.h>
#include <unordered_dense.h>

u32 chunk_hash(const u8* data, u32 len);

#define CHUNK_SIZE 32
// for use in meshing algorithms to get voxels outside of our chunk
#define CHUNK_SIZE_P (CHUNK_SIZE + 2) 

typedef enum BLOCK_TYPE {
    BLOCK_TYPE_Stone,
    BLOCK_TYPE_Dirt,
    BLOCK_TYPE_Grass,
    BLOCK_TYPE_Glass,
    BLOCK_TYPE_WoodPlank,
} BLOCK_TYPE;

typedef struct StaticBlockLookupData {
    u32 textures[6];
} StaticBlockLookupData;

const extern StaticBlockLookupData STATIC_BLOCK_LOOKUP_TABLE[];
const extern u32 STATIC_BLOCK_LOOKUP_TABLE_SIZE;

typedef struct BlockData {
    BLOCK_TYPE type;
} BlockData;

typedef struct RelBlockCoord {
    u8 block_x; 
    u8 block_y; 
    u8 block_z;
} RelBlockCoord;

typedef struct BlockCreateInfo {
    BLOCK_TYPE type;
    vec3 world_pos;
} BlockCreateInfo;

typedef struct Block {
    BlockData data;
    u8 x_rel;
    u8 y_rel;
    u8 z_rel;
    bool exists;
} Block;

typedef enum BLOCK_FACE {
    // Positive Y
    BLOCK_FACE_TOP = 0,
    // Negative Y
    BLOCK_FACE_BOT,
    // Negative X
    BLOCK_FACE_LEFT,
    // Positive X
    BLOCK_FACE_RIGHT,
    // Positive Z
    BLOCK_FACE_FRONT,
    // Negative Z
    BLOCK_FACE_BACK,
} BLOCK_FACE;

typedef enum WORLD_DIRECTION {
    WORLD_UP = 0,
    WORLD_DOWN = 1,
    WORLD_LEFT = 2,
    WORLD_RIGHT = 3,
    WORLD_FORWARD = 4,
    WORLD_BACKWARD = 5,
} WORLD_DIRECTION;

class Chunk {
    // TODO! use sparse octrees
    // Array of [CHUNK_SIZE^3] size for direct access.
    std::vector<Block> block_arr;

    // GDF_LIST of ChunkBlock pointers for easy iteration over existing blocks.
    std::vector<Block*> block_list;

    // for fast meshing
    // TODO! did nothing with this yet
    u64 axis_masks[3][CHUNK_SIZE_P][CHUNK_SIZE_P];

    // indices correspond to the BLOCK_FACE enum
    Chunk* adjacent[6];

public:
    Chunk();
    ~Chunk();

    Block* set_block(BLOCK_TYPE type, RelBlockCoord block_coord);
    Block* get_block(RelBlockCoord block_coord);
    void destroy_block(RelBlockCoord block_coord, Block* out);
};

// A terrain generator_
class Generator {
    World* world;
public:
    Generator(World* world);
    ~Generator();

    void gen_chunk(ivec3 chunk_coord, Chunk& chunk);
};

typedef struct BlockTouchingResult {
    Block* block;
    AxisAlignedBoundingBox box;
} BlockTouchingResult;

class World {
    // Terrain stuff
    Generator generator_;
    
    ankerl::unordered_dense::map<ivec3, Chunk*> chunks_;
    u8 chunk_sim_dist_;
    u16 chunk_view_dist_;

    u16 ticks_per_sec_;
    GDF_Stopwatch upd_stopwatch_;
    
    std::vector<HumanoidEntity*> humanoids_;

public:
    // temp
    PhysicsEngine physics_;

    World();
    ~World();

    void update(f64 dt);
    // Will load it from somewhere or return nullptr if nothing
    Chunk* get_chunk(ivec3 chunk_coord);
    Chunk* get_or_create_chunk(ivec3 chunk_coord);
    HumanoidEntity* create_humanoid();

    // Will not create a new chunk if it doesn't exist.
    Block* get_block(vec3 pos);
    // Will create a new chunk if it doesn't exist.
    Block* get_block_gen_chunk(vec3 pos);

    Block* set_block(BlockCreateInfo& create_info);

    void destroy_block(vec3 pos, Block* destroyed);

    // Gets the blocks that is touching an AABB.
    // Modifies the result_arr with the found blocks, and returns the amount of blocks found
    u32 get_blocks_touching(
        AxisAlignedBoundingBox* aabb,
        BlockTouchingResult* result_arr,
        u32 result_arr_size
    );
};

FORCEINLINE AxisAlignedBoundingBox block_get_aabb(vec3 world_pos) {
    return (AxisAlignedBoundingBox) {
        .min = world_pos,
        .max = vec3_add(world_pos, vec3_one())
    };
}

FORCEINLINE ivec3 world_pos_to_chunk_coord(vec3 pos) {
    // GDF_ASSERT(pos.x < i32_MAX * CHUNK_SIZE)
    // GDF_ASSERT(pos.y < i32_MAX * CHUNK_SIZE)
    // GDF_ASSERT(pos.z < i32_MAX * CHUNK_SIZE)
    return (ivec3){
        (i32)FLOOR(pos.x / CHUNK_SIZE),
        (i32)FLOOR(pos.y / CHUNK_SIZE),
        (i32)FLOOR(pos.z / CHUNK_SIZE)
    };
}

FORCEINLINE vec3 chunk_coord_to_world_pos(ivec3 coord) {
    return (vec3){
        (f32)coord.x * CHUNK_SIZE,
        (f32)coord.y * CHUNK_SIZE,
        (f32)coord.z * CHUNK_SIZE
    };
}

typedef struct ChunkBlockPosTuple {
    ivec3 cc;
    RelBlockCoord bc;
} ChunkBlockPosTuple;

FORCEINLINE ChunkBlockPosTuple world_pos_to_chunk_block_tuple(vec3 world_pos)
{
    ivec3 cc = world_pos_to_chunk_coord(world_pos);

    return (ChunkBlockPosTuple) {
        .bc = (RelBlockCoord) { 
            .block_x = (u8)(world_pos.x - cc.x * CHUNK_SIZE),
            .block_y = (u8)(world_pos.y - cc.y * CHUNK_SIZE),
            .block_z = (u8)(world_pos.z - cc.z * CHUNK_SIZE)
        },
        .cc = cc
    };
}

typedef struct WorldCreateInfo {
    u8 chunk_simulate_distance;
    u16 ticks_per_sec;
} WorldCreateInfo;

// Called every world tick by world_update()
void world_tick(World* world);