#pragma once

#include <gdfe/core.h>
#include <game/entity/humanoid.h>
#include <game/physics/engine.h>
#include <game/physics/aabb.h>
#include <gdf_math.h>
#include <unordered_dense.h>

#include "ecs.h"
#include "types.h"

class World;
u32 chunk_hash(const u8* data, u32 len);

#define CHUNK_SIZE 32
// for use in meshing algorithms to get voxels outside of our chunk
#define CHUNK_SIZE_P (CHUNK_SIZE + 2) 

typedef enum BLOCK_TYPE {
    BLOCK_TYPE_Air = 0,
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

typedef struct BlockCreateInfo {
    BLOCK_TYPE type;
    vec3 world_pos;
} BlockCreateInfo;

typedef struct Block {
    BlockData data;
    u8 x_rel;
    u8 y_rel;
    u8 z_rel;
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
    std::vector<Block> block_arr;

    // for fast meshing
    // TODO! did nothing with this yet
    u64 axis_masks[3][CHUNK_SIZE_P][CHUNK_SIZE_P];

    // indices correspond to the BLOCK_FACE enum
    Chunk* adjacent[6];

public:
    Chunk();
    ~Chunk();

    Block* set_block(BLOCK_TYPE type, u8vec3 block_coord);
    Block* get_block(u8vec3 block_coord);
    void destroy_block(u8vec3 block_coord, Block* out);
};

// A terrain generator_
class Generator {
    World* world;
public:
    Generator();
    ~Generator();

    void gen_chunk(ivec3 chunk_coord, Chunk& chunk);
};

struct WorldCreateInfo {

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

    /* Simulation stuff */
    PhysicsSimulation* physics_;
    // On the client, this will contain only the main player for client-side prediction
    // On the server, this will contain all humanoids that need to be simulated
    std::vector<SimulatedHumanoid> humanoids_;

    /* Entity component system stuff */
    ecs::Registry registry_{};
    ankerl::unordered_dense::map<ecs::Entity, u64> ecs_to_net_map_;
    ankerl::unordered_dense::map<u64, ecs::Entity> net_to_ecs_map_;

public:

    // Creates a new world with the given parameters.
    World(WorldCreateInfo& create_info);
    // Loads a world from the specified folder
    World(const char* folder_path);

    ~World();

    FORCEINLINE ecs::Registry& registry() { return registry_; }

    bool save(const char* folder_path);

    void update(f64 dt);
    // Will load it from somewhere or return nullptr if nothing
    Chunk* get_chunk(ivec3 chunk_coord) const;
    Chunk* get_or_create_chunk(ivec3 chunk_coord);
    ecs::Entity create_humanoid();

    // WILL NOT create a new chunk if it doesn't exist.
    Block* get_block(vec3 pos) const;

    Block* set_block(BlockCreateInfo& create_info);

    // Gets the blocks that is touching an AABB.
    // Modifies the result_arr with the found blocks, and returns the amount of blocks found
    const u32 get_blocks_touching(
        AxisAlignedBoundingBox* aabb,
        BlockTouchingResult* result_arr,
        u32 result_arr_size
    ) const;

    /* Utility functions for coupling the ECS entity ID and the net synced entity ID */

    // Register a relationship between a local ecs entity ID and a network synced entity ID
    FORCEINLINE void register_id_relation(const ecs::Entity ecs_id, const u64 net_id)
    {
        net_to_ecs_map_.emplace(net_id, ecs_id);
        ecs_to_net_map_.emplace(ecs_id, net_id);
    }

    // Unregister a relationship between a local ecs entity ID and a network synced entity ID,
    // using the local ecs id
    FORCEINLINE void unregister_id_relation(const ecs::Entity ecs_id)
    {
        const u64 net_id = ecs_to_net_map_.at(ecs_id);
        net_to_ecs_map_.erase(net_id);
        ecs_to_net_map_.erase(ecs_id);
    }

    // Unregister a relationship between a local ecs entity ID and a network synced entity ID,
    // using the network synced id
    FORCEINLINE void unregister_id_relation(const u64 net_id)
    {
        const ecs::Entity ecs_id = net_to_ecs_map_.at(net_id);
        ecs_to_net_map_.erase(ecs_id);
        net_to_ecs_map_.erase(net_id);
    }

    // Get the local ecs entity ID connected with the network synced entity ID
    FORCEINLINE ecs::Entity get_ecs_id(const u64 net_id)
    {
        return net_to_ecs_map_.at(net_id);
    }

    // Get the networked synced entity ID connected with the local ecs entity ID
    FORCEINLINE u64 get_net_id(const ecs::Entity ecs_id)
    {
        return ecs_to_net_map_.at(ecs_id);
    }

    // temporary
    FORCEINLINE std::vector<SimulatedHumanoid>& simulated_humanoids() { return humanoids_; }
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
    u8vec3 bc;
} ChunkBlockPosTuple;

FORCEINLINE ChunkBlockPosTuple world_pos_to_chunk_block_tuple(vec3 world_pos)
{
    ivec3 cc = world_pos_to_chunk_coord(world_pos);

    return (ChunkBlockPosTuple) {
        .bc = (u8vec3) {
            .x = (u8)(world_pos.x - cc.x * CHUNK_SIZE),
            .y = (u8)(world_pos.y - cc.y * CHUNK_SIZE),
            .z = (u8)(world_pos.z - cc.z * CHUNK_SIZE)
        },
        .cc = cc
    };
}

// Called every world tick by world_update()
void world_tick(World* world);