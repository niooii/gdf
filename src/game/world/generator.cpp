#include <game/world.h>

Generator::Generator(World* world) : world{world}
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
    RelBlockCoord bc = {
        .block_x = 0,
        .block_y = 5,
        .block_z = 0,
    };
    BLOCK_TYPE type = BLOCK_TYPE_Grass;

    for (u8 y = 0; y < 2; y++)
    {
        for (u8 x = 0; x < CHUNK_SIZE; x++)
        {
            for (u8 z = 0; z < CHUNK_SIZE; z++)
            {
                bc.block_x = x;
                bc.block_y = y;
                bc.block_z = z;

                type = (BLOCK_TYPE)0;

                chunk.set_block(type, bc);
            }
        }
    }
}
