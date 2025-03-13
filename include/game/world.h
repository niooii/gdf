#pragma once

#include <gdfe/core.h>
#include <game/entity/humanoid.h>
#include <physics/physics.h>
#include <physics/aabb.h>
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
    GDF_BOOL exists;
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
    std::array<Block, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE> block_arr;

    // GDF_LIST of ChunkBlock pointers for easy iteration over existing blocks.
    std::vector<Block*> block_list;

    // for fast meshing
    // TODO! did nothing with this yet
    u64 axis_masks[3][CHUNK_SIZE_P][CHUNK_SIZE_P];

    ivec3 cc;

    // for updating a chunk's mesh before each render only if it changes
    bool should_upd_mesh;  // replaced GDF_BOOL with standard C++ bool

    // indices correspond to the BLOCK_FACE enum
    Chunk* adjacent[6];  // Array of pointers to adjacent chunks

public:
    Chunk();

    ~Chunk();

    // You can add additional methods here to manipulate the chunk data
    // For example:
    void updateMesh();
    bool needsMeshUpdate() const;
    void setAdjacent(int face, Chunk* chunk);
    Chunk* getAdjacent(int face) const;

    // Add other functionality as needed
};

typedef struct Generator {
    u64 testfield;
} Generator;

Generator generator_create_default();

GDF_BOOL generator_gen_chunk(
    Generator* generator, 
    World* world, 
    ivec3 cc,
    Chunk* out_chunk
);

typedef struct World {
    // Terrain stuff
    Generator generator;
    
    // <ivec3, Chunk*>
    ankerl::unordered_dense::map<ivec3, Chunk*> chunks;
    // will reset every frame
    u8 chunk_simulate_distance;
    u16 chunk_view_distance;

    u16 ticks_per_sec;
    GDF_Stopwatch world_update_stopwatch;
    
    std::vector<HumanoidEntity> humanoids{32};
    
    PhysicsEngine physics;
} World;

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

GDF_BOOL chunk_init(Chunk* out_chunk);
Block* chunk_get_block(
    Chunk* chunk, 
    RelBlockCoord block_coord
);
Block* chunk_set_block(
    Chunk* chunk, 
    BLOCK_TYPE type,
    RelBlockCoord block_coord
);
void chunk_destroy_block(
    Chunk* chunk, 
    RelBlockCoord block_coord,
    Block* out
);

void world_create(World* out_world, WorldCreateInfo* create_info);
// Creates a new Humanoid Entity and returns it for modification.
HumanoidEntity* world_create_humanoid(World* world);

// Called every frame.
void world_update(World* world, f64 dt);

/* This has two behaviors:
 * If a chunk doesn't exist in RAM, it will refresh what chunks are in RAM 
 * by reading from the world's save file and try again
 * If a chunk still doesn't exist, it will create it
 */
Chunk* world_get_or_create_chunk(World* world, ivec3 coord);

// Will not create a chunk if it does not exist.  
Chunk* world_get_chunk(World* world, ivec3 coord);

// Pass NULL for destroyed_block if you don't care 
// about the properties of the destroyed block.
void world_destroy_block(World* world, vec3 block_world_pos, Block* destroyed_block);

Block* world_set_block(World* world, BlockCreateInfo* create_info);

typedef struct BlockTouchingResult {
    Block* block;
    AxisAlignedBoundingBox box;
} BlockTouchingResult;

// Gets the blocks that is touching an AABB.
// Modifies the result_arr with the found blocks, and returns the amount of blocks found
u32 world_get_blocks_touching(
    World* world, 
    AxisAlignedBoundingBox* aabb, 
    BlockTouchingResult* result_arr,
    u32 result_arr_size
);

Block* world_get_block_at(
    World* world, 
    vec3 pos
);

// Called every world tick by world_update()
void world_tick(World* world);