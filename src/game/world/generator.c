#include <game/world.h>

Generator generator_create_default()
{
    // TODO!
    Generator gen = {};

    return gen;
}

bool generator_gen_chunk(
    Generator* generator, 
    World* world, 
    ivec3 cc,
    Chunk* out_chunk
)
{
    RelBlockCoord bc = {
        .block_x = 0,
        .block_y = 5,
        .block_z = 0,
    };
    BLOCK_TYPE type = BLOCK_TYPE_Grass;

    for (u8 y = 0; y < 20; y++)
    {
        for (u8 x = 0; x < CHUNK_SIZE; x++)
        {
            for (u8 z = 0; z < CHUNK_SIZE; z++)
            {
                bc.block_x = x;
                bc.block_y = y;
                bc.block_z = z;

                type = 0;

                chunk_set_block(out_chunk, type, bc);
            }
        }
    }

    out_chunk->cc = cc;
    
    return true;
}