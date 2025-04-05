#include <game/world.h>

Chunk::Chunk()
    : block_arr(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE),
    block_list(256)
{

}

Chunk::~Chunk()
{

}

static FORCEINLINE i32 __to_idx(
    u8vec3 bc
)
{
    return bc.x + (bc.y * CHUNK_SIZE) + (bc.z * CHUNK_SIZE * CHUNK_SIZE);
}

Block* Chunk::set_block(BLOCK_TYPE type, u8vec3 block_coord)
{
    Block* block = &block_arr[
        __to_idx(
            block_coord
        )
    ];

    block->exists = true;
    block->data.type = type;
    block->x_rel = block_coord.x;
    block->y_rel = block_coord.y;
    block->z_rel = block_coord.z;

    block_list.push_back(block);

    return block;
}

Block* Chunk::get_block(u8vec3 bc)
{
    Block* chunk_block = &block_arr[
        __to_idx(
            bc
        )
    ];
    
    if (!chunk_block->exists)
        return nullptr;

    return chunk_block;
}

void Chunk::destroy_block(u8vec3 bc, Block* out)
{
    Block* block = &block_arr[
        __to_idx(
            bc
        )
    ];

    block->exists = GDF_FALSE;

    // TODO! THIS IS HORRIBLE USE A HASHMAP
    for (auto it = block_list.begin(); it != block_list.end(); ++it) {
        // TODO! bandaid fix wtf why does this happen
        if (!*it)
            continue;
        if (
            (*it)->x_rel == block->x_rel
            && (*it)->y_rel == block->y_rel
            && (*it)->z_rel == block->z_rel
        ) {
            if (out)
                *out = *(*it);
            block_list.erase(it);
            break; // Exit the loop
        }
    }
}