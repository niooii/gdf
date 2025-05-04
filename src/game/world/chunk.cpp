#include <game/world.h>

Chunk::Chunk() : block_arr(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) {}

Chunk::~Chunk() {}

static FORCEINLINE i32 __to_idx(u8vec3 bc)
{
    return bc.x + (bc.y * CHUNK_SIZE) + (bc.z * CHUNK_SIZE * CHUNK_SIZE);
}

Block* Chunk::set_block(BLOCK_TYPE type, u8vec3 block_coord)
{
    Block* block = &block_arr[__to_idx(block_coord)];

    block->data.type = type;
    block->x_rel     = block_coord.x;
    block->y_rel     = block_coord.y;
    block->z_rel     = block_coord.z;

    return block;
}

Block* Chunk::get_block(u8vec3 bc)
{
    Block* chunk_block = &block_arr[__to_idx(bc)];

    if (chunk_block->data.type == BLOCK_TYPE_Air)
        return nullptr;

    return chunk_block;
}

void Chunk::destroy_block(u8vec3 bc, Block* out)
{
    Block* block = &block_arr[__to_idx(bc)];

    // TODO! THIS IS HORRIBLE USE A HASHMAP
    for (Block& b : block_arr)
    {
        if ((b).x_rel == block->x_rel && (b).y_rel == block->y_rel && (b).z_rel == block->z_rel)
        {
            b.data.type = BLOCK_TYPE_Air;
            if (out)
                *out = b;
            break; // Exit the loop
        }
    }
}
