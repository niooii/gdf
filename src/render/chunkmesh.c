#include <render/chunkmesh.h>

// #define GDFP_DISABLE

static const VkVertexInputAttributeDescription vertex_attrs[] = {
    {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R8_UINT,
        .offset = 0
    },
    {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R8_UINT,
        .offset = 1
    },
    {
        .binding = 0,
        .location = 2,
        .format = VK_FORMAT_R8_UINT,
        .offset = 2
    },
    {
        .binding = 0,
        .location = 3,
        .format = VK_FORMAT_R8_UINT,
        .offset = 3
    },
    {
        .binding = 0,
        .location = 4,
        .format = VK_FORMAT_R16_UINT,
        .offset = 4
    },
    {
        .binding = 0,
        .location = 5,
        .format = VK_FORMAT_R8_UINT,
        .offset = 6
    },
    {
        .binding = 0,
        .location = 6,
        .format = VK_FORMAT_R8_UINT,
        .offset = 7
    },
};

void get_vertex_attrs(VkVertexInputAttributeDescription** attrs, u32* len) 
{
    *attrs = vertex_attrs;
    *len = sizeof(vertex_attrs) / sizeof(*vertex_attrs);
}

void print_mask(const char* msg, u64 n)
{
    char s[65]; 
    GDF_MemSet(s, 0, sizeof(s));
    for (u64 i = 1ULL << 63; i > 0; i = i / 2) {
        if ((n & i) != 0) {
            strcat(s, "1");
        }
        else {
            strcat(s, "0");
        }
    }
    LOG_DEBUG("%s%s", msg, s);
}

void print_mask_64(const char* msg, u64 n)
{
    char s[65]; 
    GDF_MemSet(s, 0, sizeof(s));
    for (u64 i = 1ULL << 63; i > 0; i = i / 2) {
        if ((n & i) != 0) {
            strcat(s, "1");
        }
        else {
            strcat(s, "0");
        }
    }
    LOG_DEBUG("%s%s", msg, s);
}

void print_mask_32(const char* msg, u32 n)
{
    char s[33];
    GDF_MemSet(s, 0, sizeof(s));
    for (u32 i = 1U << 31; i > 0; i = i / 2) {
        if ((n & i) != 0) {
            strcat(s, "1");
        }
        else {
            strcat(s, "0");
        }
    }
    LOG_DEBUG("%s%s", msg, s);
}

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

#define AXIS_X_ASC 0
#define AXIS_X_DESC 1
#define AXIS_Y_ASC 2
#define AXIS_Y_DESC 3
#define AXIS_Z_ASC 4
#define AXIS_Z_DESC 5

FORCEINLINE BLOCK_FACE __axis_to_face(u32 axis)
{
    switch (axis) {
        case AXIS_X_ASC:
            return BLOCK_FACE_RIGHT;
        case AXIS_X_DESC:
            return BLOCK_FACE_LEFT;
        case AXIS_Y_ASC:
            return BLOCK_FACE_TOP;
        case AXIS_Y_DESC:
            return BLOCK_FACE_BOT;
        case AXIS_Z_ASC:
            return BLOCK_FACE_FRONT;
        case AXIS_Z_DESC:
            return BLOCK_FACE_BACK;
    }
    LOG_FATAL("are u stupid");
    return -1;
}

FORCEINLINE ivec3 __get_actual_coord(u32 axis_depth, BLOCK_FACE face, u32 rel_x, u32 rel_y) 
{
    switch (face) {
        case BLOCK_FACE_TOP:
            return ivec3_new(rel_x, axis_depth + 1, rel_y);
        case BLOCK_FACE_BOT:
            return ivec3_new(rel_x, axis_depth, rel_y);
        case BLOCK_FACE_LEFT:
            return ivec3_new(axis_depth, rel_y, rel_x);
        case BLOCK_FACE_RIGHT:
            return ivec3_new(axis_depth + 1, rel_y, rel_x);
        case BLOCK_FACE_FRONT:
            return ivec3_new(rel_x, rel_y, axis_depth + 1);
        case BLOCK_FACE_BACK:
            return ivec3_new(rel_x, rel_y, axis_depth);
    }
    LOG_FATAL("are u stupid 2");
    return ivec3_zero();
}

// Implementation of a binary greedy meshing algorithm.
// WARNING: this is a very specific impl that only works for 32x32x32. take a look later..
void __mesh_chunk(ChunkMesh* mesh)
{
    #include <profiler.h>
    GDFP_START();

    World* world = mesh->world;
    Chunk* chunk = mesh->chunk;
    GDF_LIST_Clear(mesh->vertices);
    GDF_LIST_Clear(mesh->indices);

    // TODO! this could be optimized hella

    // stores the bitmasks for each axis x y and z
    u64 axis_masks[3][CHUNK_SIZE_P][CHUNK_SIZE_P];
    // stores the masks for culled faces, where 1 represents a visible face and 0 doesnt.  
    u64 axis_face_masks[6][CHUNK_SIZE_P][CHUNK_SIZE_P];

    GDF_MemSet(axis_masks, 0, sizeof(axis_masks));
    GDF_MemSet(axis_face_masks, 0, sizeof(axis_face_masks));

    // TODO! MAIN BOTTLENECK HELP
    for (i32 x = 0; x < CHUNK_SIZE_P; x++) 
    {
        for (i32 y = 0; y < CHUNK_SIZE_P; y++) 
        {
            for (i32 z = 0; z < CHUNK_SIZE_P; z++) 
            {
                vec3 world_pos = vec3_add(chunk_coord_to_world_pos(chunk->cc), vec3_new(x-1, y-1, z-1));
                
                // TODO! could replace this entire loop with just iterating through the chunks
                // owned blocks, then 8 other for loops for the outer edges of the chunk (the neighboring stuff)
                // TODO! filter blocks by visibility and solidness not existence
                if (world_get_block_at(world, world_pos)) {
                    // z,y - x axis
                    axis_masks[AXIS_X][y][z] |= 1ULL << (u64)x;
                    // x,z - y axis
                    axis_masks[AXIS_Y][z][x] |= 1ULL << (u64)y;
                    // x,y - z axis
                    axis_masks[AXIS_Z][y][x] |= 1ULL << (u64)z;
                }
            }
        }
    }
    GDFP_LOG_MSG_RESET("Finished initializing axis masks with chunk data.");

    for (u32 axis = 0; axis < 3; axis++)
    {
        for (u32 i = 0; i < CHUNK_SIZE_P; i++)
        {
            for (u32 j = 0; j < CHUNK_SIZE_P; j++)
            {
                u64 bits = axis_masks[axis][i][j];

                // TODO! does this work like this
                if (!bits)
                    continue;  
                    
                // ascending (forward) axis
                axis_face_masks[2 * axis][i][j] = (bits & ~(bits >> 1));
                // descending (backward) axis
                axis_face_masks[2 * axis + 1][i][j] = (bits & ~(bits << 1));
            
                // print_mask("asc axis: ", axis_face_masks[2 * axis][i][j]);
                // print_mask("desc axis: ", axis_face_masks[2 * axis + 1][i][j]);
            }
        }
    }
    GDFP_LOG_MSG_RESET("Finished face culling.");
    u32 cube_idx = 0;
    // One map per axis.  
    GDF_HashMap planes[6] = {
        // keys for the outer map are just the block type. 
        // each outer map is a hashmap of the pair:
        // <block_type: u32, GDF_HashMap(axis_depth: u32, plane: u32[32])> 
        // each inner map is of the pair:
        // <axis_depth: u32, plane: u32[32] (will be manually allocated when needed)>
        // TODO! might as well just use a fucking flat array oml
        // TODO! hey chat what if i make an option to create a non expandable hash table :>
        [0] = GDF_HashmapCreate(u32, GDF_HashMap(u32, u32[32]), false),
        [1] = GDF_HashmapCreate(u32, GDF_HashMap(u32, u32[32]), false),
        [2] = GDF_HashmapCreate(u32, GDF_HashMap(u32, u32[32]), false),
        [3] = GDF_HashmapCreate(u32, GDF_HashMap(u32, u32[32]), false),
        [4] = GDF_HashmapCreate(u32, GDF_HashMap(u32, u32[32]), false),
        [5] = GDF_HashmapCreate(u32, GDF_HashMap(u32, u32[32]), false),
    };
    // maintain a list of allocated planes for easier (arguably faster - should benchmark) deallocation 
    GDF_LIST(u32*) allocated_planes = GDF_LIST_Reserve(u32*, 256);

    for (u32 axis = 0; axis < 6; axis++) 
    {
        for (u32 i = 0; i < CHUNK_SIZE; i++)
        {
            for (u32 j = 0; j < CHUNK_SIZE; j++)
            {
                // skip neighboring chunk masks
                u64 bits = axis_face_masks[axis][i + 1][j + 1];

                // remove the rightmost and leftmost bit, we dont care about those
                bits >>= 1;
                bits &= ~(1ULL << CHUNK_SIZE);

                while (bits != 0) 
                {
                    // where the face is (kinda)
                    // more like how deep it is along the axis
                    u8 depth = __builtin_ctz(bits);
                    bits &= bits - 1;

                    RelBlockCoord block_coord;
                    switch(axis) {
                        case AXIS_X_ASC:
                        case AXIS_X_DESC:
                            block_coord.block_x = depth;
                            block_coord.block_y = i;
                            block_coord.block_z = j;
                        break;
                        case AXIS_Y_ASC:
                        case AXIS_Y_DESC:
                            block_coord.block_x = j;
                            block_coord.block_y = depth;
                            block_coord.block_z = i;
                        break;
                        case AXIS_Z_ASC:
                        case AXIS_Z_DESC:
                            block_coord.block_x = j;
                            block_coord.block_y = i;
                            block_coord.block_z = depth;
                        break;
                    }

                    Block* block = chunk_get_block(chunk, block_coord);

                    if (!block) {
                        LOG_ERR("yea yo code is doodoo");
                    }

                    GDF_HashMap* depth_map_p = GDF_HashmapGet(planes[axis], &block->data.type);
                    if (!depth_map_p) {
                        GDF_HashMap t = GDF_HashmapCreate(u32, u32*, false);
                        depth_map_p = GDF_HashmapInsert(
                            planes[axis], 
                            &block->data.type, 
                            &t,
                            NULL
                        );
                        GDF_ASSERT(depth_map_p);
                    }

                    GDF_HashMap depth_map = *depth_map_p;

                    u32** plane_p = GDF_HashmapGet(depth_map, &depth);
                    if (!plane_p) {
                        u32* plane = GDF_Malloc(sizeof(u32) * 32, GDF_MEMTAG_ARRAY);
                        plane_p = GDF_HashmapInsert(depth_map, &depth, &plane, NULL);
                        GDF_LIST_Push(allocated_planes, plane);
                    }

                    // 32 size
                    u32* plane = *plane_p;
                    plane[j] |= (u32)1 << i;
                }
            }
        }
    }
    GDFP_LOG_MSG_RESET("Finished generating binary planes.");

    // iterate for each axis (forward and backward), then for each block type in the map we get all the planes that
    // need to be meshed at a specific depth. 
    for (u32 axis = 0; axis < 6; axis++) {
        BLOCK_FACE face = __axis_to_face(axis);
        for (
            HashmapEntry* type_map = GDF_HashmapIter(planes[axis]);
            type_map != NULL;
            GDF_HashmapIterAdvance(&type_map)
        ) {
            // no type chekcing is so scary help m,e
            u32 block_type = *(u32*)type_map->key;
            GDF_HashMap depth_map = *(GDF_HashMap*)type_map->val;
            
            for (
                HashmapEntry* depth_entry = GDF_HashmapIter(depth_map);
                depth_entry != NULL;
                GDF_HashmapIterAdvance(&depth_entry)
            ) {
                u32 depth = *(u32*)depth_entry->key;
                u32* plane = *(u32**)depth_entry->val;

                // actual greedy meshing algorithim
                // debugging stuff
                // for (int k = 0; k < 32; k++) {
                //     print_mask_32("mask: ", plane[k]);
                // }
                for (u32 row = 0; row < CHUNK_SIZE; row++) {
                    // x is the current x position within this row
                    u32 x = 0;
                    while (x < CHUNK_SIZE) {
                        // TODO! could we pregenerate this per chunk and then just update a few bits
                        // on each chunk change, then deep copy the data in this function?
                        // im p sure itll be faster
                        x += __builtin_ctz((plane[row] >> x));
                        if (x >= CHUNK_SIZE)
                            continue;

                        u32 w = __builtin_ctz(~(plane[row] >> x));
                        // print_mask_32("mask: ", plane[row]);
                        // LOG_DEBUG("width: %d", w);
                        // account for overflow (where shifting is undefined SCARY!!)
                        u32 w_mask = ((w >= 32) ? 0xFFFFFFFF : ((1ULL << w) - 1)) << x;

                        u32 h = 1;
                        // if you ever change chunk size beware of this
                        while (row + h < CHUNK_SIZE) {
                            if ((w_mask & plane[row + h]) != w_mask)
                                break;

                            // set all the processed bits to 0
                            plane[row + h] &= ~w_mask;

                            h++;
                        }

                        // push the vertices for the quad we just made
                        // LOG_DEBUG("depth: %d", depth);
                        ivec3 v0_pos = __get_actual_coord(depth, face, row, x);
                        ivec3 v1_pos = __get_actual_coord(depth, face, row + h, x);
                        ivec3 v2_pos = __get_actual_coord(depth, face, row + h, x + w);
                        ivec3 v3_pos = __get_actual_coord(depth, face, row, x + w);
                        ChunkVertex v0 = {
                            .block_type = block_type,
                            .face_dir = face,
                            .x_pos = v0_pos.x,
                            .y_pos = v0_pos.y,
                            .z_pos = v0_pos.z,
                            .u = 0,
                            .v = 0
                        };
                        ChunkVertex v1 = {
                            .block_type = block_type,
                            .face_dir = face,
                            .x_pos = v1_pos.x,
                            .y_pos = v1_pos.y,
                            .z_pos = v1_pos.z,
                            .u = h,
                            .v = 0
                        };
                        ChunkVertex v2 = {
                            .block_type = block_type,
                            .face_dir = face,
                            .x_pos = v2_pos.x,
                            .y_pos = v2_pos.y,
                            .z_pos = v2_pos.z,
                            .u = h,
                            .v = w
                        };
                        ChunkVertex v3 = {
                            .block_type = block_type,
                            .face_dir = face,
                            .x_pos = v3_pos.x,
                            .y_pos = v3_pos.y,
                            .z_pos = v3_pos.z,
                            .u = 0,
                            .v = w
                        };
                        u32 num_vertices = GDF_LIST_GetLength(mesh->vertices);
                        GDF_LIST_Push(mesh->vertices, v0);
                        GDF_LIST_Push(mesh->vertices, v1);
                        GDF_LIST_Push(mesh->vertices, v2);
                        GDF_LIST_Push(mesh->vertices, v3);
                        
                        // push indices based on face dir for consistent winding
                        switch (face) {
                            case BLOCK_FACE_FRONT: // +Z face
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 3); // v3
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices + 1); // v1
                                break;
                            
                            case BLOCK_FACE_BACK: // -Z face
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 1); // v1
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices + 3); // v3
                                break;
                            
                            case BLOCK_FACE_RIGHT: // +X face
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 1); // v1
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices + 3); // v3
                                break;
                            
                            case BLOCK_FACE_LEFT: // -X face
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 3); // v3
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices + 1); // v1
                                break;
                            
                            case BLOCK_FACE_TOP: // +Y face
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 1); // v1
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices + 3); // v3
                                break;
                            
                            case BLOCK_FACE_BOT: // -Y face
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 3); // v3
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices);     // v0
                                GDF_LIST_Push(mesh->indices, num_vertices + 2); // v2
                                GDF_LIST_Push(mesh->indices, num_vertices + 1); // v1
                                break;
                        }

                        x += w;
                    }
                }
            }
        }
    }

    GDFP_LOG_MSG_RESET("Finished meshing algorithm.");

    // finalization
    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        mesh->buffers[i].up_to_date = false;
    }
    // __gen_indices(mesh);
    mesh->num_indices = GDF_LIST_GetLength(mesh->indices);

    // cleanup
    for (u32 i = 0; i < 6; i++) {
        for (
            HashmapEntry* iter = GDF_HashmapIter(planes[i]);
            iter != NULL;
            GDF_HashmapIterAdvance(&iter)
        ) {
            GDF_HashMap inner = *(GDF_HashMap*)iter->val;
            GDF_ASSERT(GDF_HashmapDestroy(inner));
        }
        GDF_ASSERT(GDF_HashmapDestroy(planes[i]));
    }

    u32 num_planes = GDF_LIST_GetLength(allocated_planes);
    for (u32 i = 0; i < num_planes; i++) {
        GDF_Free(allocated_planes[i]);
    }
    GDF_LIST_Destroy(allocated_planes);

    GDFP_END();
}

// mesh should not be accessed while its being initialized. 
bool chunk_mesh_init(VkRenderContext* ctx, World* world, Chunk* chunk, ChunkMesh* mesh) 
{
    // BUG! untested
    GDF_MemSet(mesh, 0, sizeof(*mesh));
    mesh->chunk = chunk;
    mesh->world = world;

    mesh->vertices = GDF_LIST_Create(ChunkVertex);
    mesh->indices = GDF_LIST_Create(CHUNK_MESH_INDEX_TYPE);
    __mesh_chunk(mesh);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        GDF_ASSERT_RETURN_FALSE(
            buffers_create_vertex(
                ctx,
                NULL,
                MAX_CHUNK_VERTICES,
                sizeof(ChunkVertex),
                &mesh->buffers[i].vertex_buffer
            )
        );
        GDF_ASSERT_RETURN_FALSE(
            buffers_create_index(
                ctx,
                NULL,
                MAX_CHUNK_INDICES,
                &mesh->buffers[i].index_buffer
            )
        );
        // update info for all buffers in the init pass as well.
        if (!chunk_mesh_update_buffers(ctx, mesh, i)) {
            LOG_FATAL("Failed to update chunkmesh buffer");
            return false;
        }
    }
    return true;
}

bool chunk_mesh_update_buffers(VkRenderContext* ctx, ChunkMesh* mesh, u16 i)
{
    GDF_ASSERT_RETURN_FALSE(
        buffers_update(
            ctx, 
            &mesh->buffers[i].vertex_buffer, 
            mesh->vertices,
            GDF_LIST_GetLength(mesh->vertices) * sizeof(ChunkVertex)
        )
    );
    GDF_ASSERT_RETURN_FALSE(
        buffers_update(
            ctx, 
            &mesh->buffers[i].index_buffer, 
            mesh->indices,
            mesh->num_indices * sizeof(CHUNK_MESH_INDEX_TYPE)  
        )
    );
    mesh->buffers[i].up_to_date = true;
    return true;
}

bool chunk_mesh_update(ChunkMesh* mesh, ChunkMeshUpdates* updates) 
{
    // wow chat. 
    __mesh_chunk(mesh);
    return true;
}

void chunk_mesh_destroy(VkRenderContext* ctx, ChunkMesh* mesh) 
{
    GDF_LIST_Destroy(mesh->vertices);
    GDF_LIST_Destroy(mesh->indices);

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        buffers_destroy(
            ctx, 
            &mesh->buffers[i].vertex_buffer 
        );
        buffers_destroy(
            ctx, 
            &mesh->buffers[i].index_buffer 
        );
    }
}
