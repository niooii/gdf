#pragma once

#include <../../gdfe/include/core.h>
#include <game/world.h>
#include <render/vk_types.h>

#include "graphics/renderer.h"

// holy wasteful man
#define MAX_CHUNK_VERTICES (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 8)
#define MAX_CHUNK_INDICES (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 36)
#define CHUNK_MESH_INDEX_TYPE u16

struct ChunkVertex_T {
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
} __attribute__((packed));

typedef struct ChunkVertex_T ChunkVertex;

typedef struct MeshBuffer {
    GDF_VkBuffer vertex_buffer;
    GDF_VkBuffer index_buffer;

    bool up_to_date;
} MeshBuffer;

typedef struct ChunkMesh {
    GDF_LIST(ChunkVertex) vertices;
    GDF_LIST(CHUNK_MESH_INDEX_TYPE) indices;
    
    // for concurrency safety
    // will be updated with the frames in flight fences
    MeshBuffer buffers[MAX_FRAMES_IN_FLIGHT];
    
    VkDescriptorSet descriptor_set;

    u32 num_indices;
    
    bool is_visible; 

    // rust has made me scared of stuff like this
    // TODO! RW LOCK PLEASE!!
    Chunk* chunk;
    World* world;
} ChunkMesh;

// Changes per frame can be batched and updated all at once.  
typedef struct ChunkMeshUpdates {
    GDF_HashMap(ivec3, Block*) created;
} ChunkMeshUpdates; 

bool chunk_mesh_init(GDF_VkRenderContext* ctx, World* world, Chunk* chunk, ChunkMesh* mesh);
// for now this shit rebuilds the entire chunk lol but  please use it as implmeneted
bool chunk_mesh_update(ChunkMesh* mesh, ChunkMeshUpdates* updates);
void chunk_mesh_destroy(GDF_VkRenderContext* ctx, ChunkMesh* mesh);

bool chunk_mesh_update_buffers(GDF_VkRenderContext* ctx, ChunkMesh* mesh, u16 buffer_idx);

// no memory should be allocated, the data is static
void get_vertex_attrs(VkVertexInputAttributeDescription** attrs, u32* len);