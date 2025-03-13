#pragma once

#include <gdfe/core.h>
#include <game/world.h>
#include <gdfe/render/vk_types.h>

#include "graphics/renderer.h"

// holy wasteful man
#define MAX_CHUNK_VERTICES (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 8)
#define MAX_CHUNK_INDICES (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 36)
#define CHUNK_MESH_INDEX_TYPE u16

PACKED_STRUCT ChunkVertex_T {
    u8 x_pos;
    u8 y_pos;
    u8 z_pos;
    
    // BLOCK_FACE enum
    u8 face_dir;
    u16 block_type;
    
    // u16 face_idx;
    u8 u;
    u8 v;
    
    // uint8_t light_level;
    // uint8_t ao_level;
} END_PACKED_STRUCT;

typedef struct ChunkVertex_T ChunkVertex;

typedef struct MeshBuffer {
    GDF_VkBuffer vertex_buffer;
    GDF_VkBuffer index_buffer;

    bool up_to_date;
} MeshBuffer;

class ChunkMesh {
    std::vector<ChunkVertex> vertices_;
    std::vector<CHUNK_MESH_INDEX_TYPE> indices_;

    // TODO! should really not store buffers here. update them
    // in the render callback
    MeshBuffer buffers_[MAX_FRAMES_IN_FLIGHT] = {};
    Chunk* chunk_;
    // TODO! remove these in the future design it better
    World* world_;
    ivec3 chunk_coord_;

public:
    // TODO! make it use adjacent chunk data not
    // requiring the entire world to be passed in
    ChunkMesh(World* world, Chunk* chunk, ivec3 chunk_coord);
    ~ChunkMesh();

    void mesh();
    bool update_buffers(u32 frame_idx);
    MeshBuffer* get_mesh_buffer(u32 frame_idx);
};

// Changes per frame can be batched and updated all at once.
typedef struct ChunkMeshUpdates {
    // GDF_HashMap(ivec3, Block*) created;
} ChunkMeshUpdates; 

GDF_BOOL chunk_mesh_init(GDF_VkRenderContext* ctx, World* world, Chunk* chunk, ChunkMesh* mesh);
// for now this shit rebuilds the entire chunk lol but  please use it as implmeneted
GDF_BOOL chunk_mesh_update(ChunkMesh* mesh, ChunkMeshUpdates* updates);
void chunk_mesh_destroy(GDF_VkRenderContext* ctx, ChunkMesh* mesh);

GDF_BOOL chunk_mesh_update_buffers(GDF_VkRenderContext* ctx, ChunkMesh* mesh, u16 buffer_idx);

// no memory should be allocated, the data is static
void get_vertex_attrs(VkVertexInputAttributeDescription** attrs, u32* len);