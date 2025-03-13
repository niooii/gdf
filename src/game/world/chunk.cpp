#include <game/world.h>

Chunk::Chunk()
    : block_arr(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
{

}

Chunk::~Chunk()
{

}

GDF_BOOL chunk_init(Chunk* out_chunk)
{
    out_chunk->block_arr;
    // out_chunk->block_list = (Block**)GDF_LIST_Reserve(Block*, 2048);

    out_chunk->should_upd_mesh = GDF_TRUE;
    GDF_MemSet(out_chunk->axis_masks, 0, sizeof(out_chunk->axis_masks));

    // out_chunk->faces = NULL;
    return GDF_TRUE;
}

static FORCEINLINE i32 __to_idx(
    RelBlockCoord bc
)
{
    return bc.block_x + (bc.block_y * CHUNK_SIZE) + (bc.block_z * CHUNK_SIZE * CHUNK_SIZE);
}

Block* chunk_get_block(
    Chunk* chunk, 
    RelBlockCoord bc
)
{
    Block* chunk_block = &chunk->block_arr[
        __to_idx(
            bc
        )
    ];
    
    if (!chunk_block->exists)
        return NULL;
    else
        return chunk_block;
}

Block* chunk_set_block(
    Chunk* chunk, 
    BLOCK_TYPE type,
    RelBlockCoord block_coord
)
{
    Block* block = &chunk->block_arr[
        __to_idx(
            block_coord
        )
    ];

    block->exists = GDF_TRUE;
    block->x_rel = block_coord.block_x;
    block->y_rel = block_coord.block_y;
    block->z_rel = block_coord.block_z;

    block->data.type = type;

    GDF_LIST_Push(chunk->block_list, block);

    return block;
}

void chunk_destroy_block(
    Chunk* chunk, 
    RelBlockCoord bc,
    Block* out
)
{
    Block* block = &chunk->block_arr[
        __to_idx(
            bc
        )
    ];
    block->exists = GDF_FALSE;
    // TODO! THIS IS HORRIBLE USE A HASHMAP
    u64 len = GDF_LIST_GetLength(chunk->block_list);
    for (i32 i = 0; i < len; i++)
    {
        Block* b = chunk->block_list[i]; 
        if (
            block == b
        )
        {
            GDF_LIST_Remove(chunk->block_list, i, out);
            break;
        }
    }
}