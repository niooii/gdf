#include <prelude.h>

#include <game/world.h>

Generator::Generator()
{

}

Generator::~Generator()
{

}

void Generator::gen_chunk(ivec3 cc, Chunk& chunk)
{
    // quick checkerboard patter with chunks lol
    // nah nvm
    // if (cc.x % 2 == (cc.y % 2) || cc.z % 2 == ((cc.y + 1) % 2))
    // {
    //     return;
    // }
    u8vec3 bc = {
        .x = 0,
        .y = 5,
        .z = 0,
    };
    BLOCK_TYPE type = BLOCK_TYPE_Grass;

    for (u8 y = 0; y < 2; y++)
    {
        for (u8 x = 0; x < CHUNK_SIZE; x++)
        {
            for (u8 z = 0; z < CHUNK_SIZE; z++)
            {
                bc.x = x;
                bc.y = y;
                bc.z = z;

                type = (BLOCK_TYPE)1;

                chunk.set_block(type, bc);
            }
        }
    }
}
