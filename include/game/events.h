#pragma once

#include <../../gdfe/include/core.h>
#include <event.h>
#include <game/world.h>

// TODO! kill this
typedef enum GDF_EVENT {
    /*
    A chunk was modified
    Usage:
    Chunk* chunk = (Chunk*)sender;
    */
    GDF_EVENT_CHUNK_UPDATE = GDF_EVENT_INTERNAL_MAX + 1,
    /*
    Whenever a chunk is loaded or created.
    Usage:
    Chunk* chunk = (Chunk*)sender;
    */
    GDF_EVENT_CHUNK_LOAD,
    /*
    Fires if an Entity touches a block.
    Usage:
    Block* block = (Block*)sender
    Entity* h = (Entity*)(ctx.data.u64[0])
    */
    GDF_EVENT_BLOCK_TOUCHED,
    /*
    Whenever a chunk is unloaded.
    The chunk pointer is guarenteed to be valid when the event
    handlers are called. 
    Usage:
    Chunk* chunk = (Chunk*)sender;
    */
    GDF_EVENT_CHUNK_UNLOAD,
    
    // Not actaully an event.
    GDF_EVENT_MAX = 0xFFF

} GDF_EVENT; 

#define CHUNK_LOAD_HANDLER void (*handle_chunk_load)(World* world, Chunk* chunk, void* state)
void on_chunk_load(CHUNK_LOAD_HANDLER);

typedef struct ChunkUpdates {
    // Hashmap of <RelBlockCoord, Block*>.
    // If the block was removed, then the block pointer will be NULL.
    GDF_HashMap(RelBlockCoord, Block*) updated_blocks;
} ChunkUpdates;

#define CHUNK_UPDATE_HANDLER void (*handle_chunk_update)(World* world, Chunk* chunk, ChunkUpdates* updates, void* state)
void on_chunk_update(CHUNK_UPDATE_HANDLER);

#define CHUNK_UNLOAD_HANDLER void (*handle_chunk_unload)(World* world, Chunk* chunk, void* state)
void on_chunk_unload(CHUNK_UNLOAD_HANDLER);

#define TICK_HANDLER void (*handle_tick)(World* world, u64 tick_number, void* state)
void on_tick(TICK_HANDLER);

// TODO! remove handlers function uhhh