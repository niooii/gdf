#include <client/graphics/textures.h>
#include <game/world.h>

const StaticBlockLookupData STATIC_BLOCK_LOOKUP_TABLE[] = {
    [BLOCK_TYPE_Grass]     = { .textures = { [BLOCK_FACE_TOP] = GDF_TEXTURE_INDEX_GRASS_TOP,
                                   [BLOCK_FACE_BOT]           = GDF_TEXTURE_INDEX_DIRT,
                                   [BLOCK_FACE_LEFT]          = GDF_TEXTURE_INDEX_GRASS_SIDE,
                                   [BLOCK_FACE_RIGHT]         = GDF_TEXTURE_INDEX_GRASS_SIDE,
                                   [BLOCK_FACE_FRONT]         = GDF_TEXTURE_INDEX_GRASS_SIDE,
                                   [BLOCK_FACE_BACK]          = GDF_TEXTURE_INDEX_GRASS_SIDE } },
    [BLOCK_TYPE_Dirt]      = { .textures = { [BLOCK_FACE_TOP] = GDF_TEXTURE_INDEX_DIRT,
                                   [BLOCK_FACE_BOT]           = GDF_TEXTURE_INDEX_DIRT,
                                   [BLOCK_FACE_LEFT]          = GDF_TEXTURE_INDEX_DIRT,
                                   [BLOCK_FACE_RIGHT]         = GDF_TEXTURE_INDEX_DIRT,
                                   [BLOCK_FACE_FRONT]         = GDF_TEXTURE_INDEX_DIRT,
                                   [BLOCK_FACE_BACK]          = GDF_TEXTURE_INDEX_DIRT } },
    [BLOCK_TYPE_Stone]     = { .textures = { [BLOCK_FACE_TOP] = GDF_TEXTURE_INDEX_STONE,
                                   [BLOCK_FACE_BOT]           = GDF_TEXTURE_INDEX_STONE,
                                   [BLOCK_FACE_LEFT]          = GDF_TEXTURE_INDEX_STONE,
                                   [BLOCK_FACE_RIGHT]         = GDF_TEXTURE_INDEX_STONE,
                                   [BLOCK_FACE_FRONT]         = GDF_TEXTURE_INDEX_STONE,
                                   [BLOCK_FACE_BACK]          = GDF_TEXTURE_INDEX_STONE } },
    [BLOCK_TYPE_WoodPlank] = { .textures = { [BLOCK_FACE_TOP] = GDF_TEXTURE_INDEX_WOOD_PLANK,
                                   [BLOCK_FACE_BOT]           = GDF_TEXTURE_INDEX_WOOD_PLANK,
                                   [BLOCK_FACE_LEFT]          = GDF_TEXTURE_INDEX_WOOD_PLANK,
                                   [BLOCK_FACE_RIGHT]         = GDF_TEXTURE_INDEX_WOOD_PLANK,
                                   [BLOCK_FACE_FRONT]         = GDF_TEXTURE_INDEX_WOOD_PLANK,
                                   [BLOCK_FACE_BACK]          = GDF_TEXTURE_INDEX_WOOD_PLANK } },
};

const u32 STATIC_BLOCK_LOOKUP_TABLE_SIZE = sizeof(STATIC_BLOCK_LOOKUP_TABLE);
