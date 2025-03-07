#include <game/world.h>
#include <game/events.h>
#include <render/renderer.h>

u32 chunk_hash(const u8* data, u32 len) {
    ivec3* coord = (ivec3*)(data);

    // primes n stuff
    const u32 p1 = 73856093u;  
    const u32 p2 = 19349663u;
    const u32 p3 = 83492791u;

    u32 h1 = (u32)(coord->x) * p1;
    u32 h2 = (u32)(coord->y) * p2;
    u32 h3 = (u32)(coord->z) * p3;

    return h1 ^ h2 ^ h3;
}
//
// static bool __on_chunk_load(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx)
// {
//     LOG_DEBUG("CHUNK LOAD");
//     World* world = (World*)listener_instance;
//     Chunk* chunk = (Chunk*)sender;
//
//     renderer_register_chunk(chunk);
//     for (u32 i = 0; i < 6; i++) {
//         ivec3 adj = ivec3_add(chunk->cc, ivec3_adjacent_offsets[i]);
//         renderer_queue_chunk_remesh(adj);
//     }
//
//     return false;
// }
//
// static bool __on_chunk_unload(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx)
// {
//     LOG_DEBUG("CHUNK UNLOAD...");
//     World* world = (World*)listener_instance;
//
//     Chunk* chunk = (Chunk*)sender;
//     renderer_queue_chunk_remesh(chunk->cc);
//
//     GDF_ASSERT(renderer_remove_chunk(chunk->cc));
//
//     return false;
// }
//
// // TODO!
// static bool __on_chunk_update(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx)
// {
//     LOG_DEBUG("CHUNK UPDATE");
//     World* world = (World*)listener_instance;
//     Chunk* chunk = (Chunk*)sender;
//
//     renderer_queue_chunk_remesh(chunk->cc);
//
//     return false;
// }

static bool __on_block_touch(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx)
{
    World* world = (World*)listener_instance;
    Block* block = (Block*)sender;
    Entity* entity = (Entity*)(ctx.data.u64[0]);
    if (entity->type != ENTITY_TYPE_HUMANOID)
        return false;

    HumanoidEntity* hum = entity->parent;

    return false;
}

Chunk* __gen_chunk(World* world, ivec3 coord)
{
    Chunk** cp = (Chunk**)GDF_HashmapGet(world->chunks, &coord);
    if (cp == NULL)
    {
        Chunk* t = GDF_Malloc(sizeof(Chunk), GDF_MEMTAG_GAME);
        chunk_init(t);
        cp = GDF_HashmapInsert(world->chunks, &coord, &t, NULL);
        if (cp == NULL)
        {
            LOG_WARN("WOMP WOMP");
        } 
        
        if (!generator_gen_chunk(&world->generator, world, coord, *cp))
        {
            LOG_ERR("Failed to create chunk");
            return NULL;
        }
    }

    return *cp;
}

void world_create(World* out_world, WorldCreateInfo* create_info)
{
    PhysicsCreateInfo physics_info = {
        .gravity = {0, -20, 0},
        .gravity_active = true,
        .air_drag = 3.f,
        .ground_drag = 12.f,
        .terminal_velocity = -50.f
    };
    out_world->physics = physics_init(physics_info);
    out_world->chunk_simulate_distance = create_info->chunk_simulate_distance;
    i32 chunk_sim_distance = out_world->chunk_simulate_distance;
    out_world->ticks_per_sec = create_info->ticks_per_sec;
    out_world->chunks = GDF_HashmapWithHasher(ivec3, Chunk*, chunk_hash, false);
    out_world->humanoids = GDF_LIST_Reserve(HumanoidEntity*, 32);

    out_world->world_update_stopwatch = GDF_StopwatchCreate();

    GDF_LIST(Chunk*) created_chunks = GDF_LIST_Reserve(Chunk*, 128);

    // Create chunks
    for (i32 chunk_x = -4; chunk_x <= 4; chunk_x++)
    {
        for (i32 chunk_y = 0; chunk_y < 1; chunk_y++)
        {
            for (i32 chunk_z = -4; chunk_z <= 4; chunk_z++)
            {
                ivec3 cc = {
                    .x = chunk_x,
                    .y = chunk_y,
                    .z = chunk_z
                };
                Chunk* c = __gen_chunk(out_world, cc);
                GDF_LIST_Push(created_chunks, c);
            }
        }
    }

    u32 num_chunks = GDF_LIST_GetLength(created_chunks);
    for (u32 i = 0; i < num_chunks; i++) 
    {
        // renderer_register_chunk(created_chunks[i]);
    }

    // TODO! make events a SET of listeners not a list no problem
    // GDF_ASSERT(GDF_EventRegister(GDF_EVENT_BLOCK_TOUCHED, out_world, __on_block_touch));
    // GDF_ASSERT(GDF_EventRegister(GDF_EVENT_CHUNK_LOAD, out_world, __on_chunk_load));
    // GDF_ASSERT(GDF_EventRegister(GDF_EVENT_CHUNK_UNLOAD, out_world, __on_chunk_unload));
    // GDF_ASSERT(GDF_EventRegister(GDF_EVENT_CHUNK_UPDATE, out_world, __on_chunk_update));
}

HumanoidEntity* world_create_humanoid(World* world)
{
    HumanoidEntity* hum = GDF_Malloc(sizeof(HumanoidEntity), GDF_MEMTAG_GAME);

    GDF_LIST_Push(world->humanoids, hum);

    hum->base.type = ENTITY_TYPE_HUMANOID;
    hum->base.parent = hum;

    return hum;
}

// if multiplayer this should only run on the client. god i want to die
void world_update(World* world, f64 dt)
{
    u32 num_humanoids = GDF_LIST_GetLength(world->humanoids);
    for (u32 i = 0; i < num_humanoids; i++)
        humanoid_entity_update(world->humanoids[i]);
    physics_update(world->physics, world, dt);

    // here we remesh chunks if they changed
    // TODO! a list of chunks ptrs may be bad for cache locality benchmark a bit later
    // u32 len = GDF_LIST_GetLength(world->queued_mesh_upds);

    // for (u32 i = 0; i < len; i++) {
    //     Chunk* changed = world->queued_mesh_upds;
    //     renderer_update_chunk(changed);
    // }
    // GDF_LIST_Clear(world->queued_mesh_upds);
}

Chunk* world_get_or_create_chunk(World* world, ivec3 coord)
{
    Chunk** cp = (Chunk**)GDF_HashmapGet(world->chunks, &coord);
    if (cp) 
        return *cp;

    Chunk* c = __gen_chunk(world, coord);
    if (!c)
        return NULL;

    GDF_EventFire(GDF_EVENT_CHUNK_LOAD, c, (GDF_EventContext){});

    return c;
}

Chunk* world_get_chunk(World* world, ivec3 coord)
{
    Chunk** cp = (Chunk**)GDF_HashmapGet(world->chunks, &coord);
    if (cp == NULL)
        return NULL;

    return *cp;
}

// TODO! what about usign teh chunk api directly man..
void world_destroy_block(World* world, vec3 block_world_pos, Block* destroyed_block)
{
    ChunkBlockPosTuple info = world_pos_to_chunk_block_tuple(block_world_pos);
    Chunk* c = world_get_or_create_chunk(world, info.cc);
    chunk_destroy_block(c, info.bc, NULL);

    // TODO! remove this please find a way to fit this inside the chunk functions directly.
    // or better yet through the use of actual structured event data. PLEASE!!
    // OR MAYBE JUST MAEK THE EVENT SYSTEM TO POOL CHANGES . YES.,
    RelBlockCoord bc = info.bc;

    GDF_EventFire(GDF_EVENT_CHUNK_UPDATE, c, (GDF_EventContext){});

    // if (bc.block_x > 0 && bc.block_x < CHUNK_SIZE - 1
    //     && bc.block_y > 0 && bc.block_y < CHUNK_SIZE - 1
    //     && bc.block_z > 0 && bc.block_z < CHUNK_SIZE - 1)  {
    //     return;
    // }
    // if (bc.block_x == 0) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(-1, 0, 0)));
    // } else if (bc.block_x == CHUNK_SIZE - 1) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(1, 0, 0)));
    // }
    //
    // if (bc.block_y == 0) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(0, -1, 0)));
    // } else if (bc.block_y == CHUNK_SIZE - 1) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(0, 1, 0)));
    // }
    //
    // if (bc.block_z == 0) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(0, 0, -1)));
    // } else if (bc.block_z == CHUNK_SIZE - 1) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(0, 0, 1)));
    // }
}

Block* world_set_block(World* world, BlockCreateInfo* create_info)
{
    ChunkBlockPosTuple info = world_pos_to_chunk_block_tuple(create_info->world_pos);
    Chunk* c = world_get_or_create_chunk(world, info.cc);
    Block* b = chunk_set_block(c, create_info->type, info.bc);

    // TODO! remove this please find a way to fit this inside the chunk functions directly.
    // or better yet through the use of actual structured event data. PLEASE!!
    RelBlockCoord bc = info.bc;

    GDF_EventFire(GDF_EVENT_CHUNK_UPDATE, c, (GDF_EventContext){});

    // if (bc.block_x > 0 && bc.block_x < CHUNK_SIZE - 1
    //     && bc.block_y > 0 && bc.block_y < CHUNK_SIZE - 1
    //     && bc.block_z > 0 && bc.block_z < CHUNK_SIZE - 1)  {
    //     return b;
    // }
    // if (bc.block_x == 0) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(-1, 0, 0)));
    // } else if (bc.block_x == CHUNK_SIZE - 1) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(1, 0, 0)));
    // }
    //
    // if (bc.block_y == 0) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(0, -1, 0)));
    // } else if (bc.block_y == CHUNK_SIZE - 1) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(0, 1, 0)));
    // }
    //
    // if (bc.block_z == 0) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(0, 0, -1)));
    // } else if (bc.block_z == CHUNK_SIZE - 1) {
    //     renderer_queue_chunk_remesh(ivec3_add(info.cc, ivec3_new(0, 0, 1)));
    // }
    
    return b;
}

u32 world_get_blocks_touching(
    World* world, 
    AxisAlignedBoundingBox* aabb, 
    BlockTouchingResult* result_arr,
    u32 result_arr_size
)
{
    f32 min_x = FLOOR(aabb->min.x);
    f32 min_y = FLOOR(aabb->min.y);
    f32 min_z = FLOOR(aabb->min.z);
    f32 max_x = FLOOR(aabb->max.x);
    f32 max_y = FLOOR(aabb->max.y);
    f32 max_z = aabb->max.z;
    u32 i = 0;
    Block* cb = NULL;
    for (f32 x = min_x; x <= max_x; x++)
    {
        for (f32 y = min_y; y <= max_y; y++)
        {
            for (f32 z = min_z; z <= max_z; z++)
            {
                cb = world_get_block_at(world, vec3_new(x, y, z));
                if (cb == NULL)
                    continue;
                
                BlockTouchingResult* res = &result_arr[i++];
                res->block = cb;
                res->box = (AxisAlignedBoundingBox) {
                    .min = vec3_new(x, y, z),
                    .max = vec3_new(x+1, y+1, z+1)
                };

                if (i >= result_arr_size)
                    return i;
            }
        }
    }
    return i;
}

Block* world_get_block_at(
    World* world, 
    vec3 pos
)
{
    ChunkBlockPosTuple tuple = world_pos_to_chunk_block_tuple(pos);
    Chunk* chunk = world_get_chunk(world, tuple.cc);
    if (chunk == NULL)
        return NULL;
    
    return chunk_get_block(chunk, tuple.bc);
}

Block* world_get_block_if_exists(
    World* world, 
    vec3 pos
)
{
    ChunkBlockPosTuple tuple = world_pos_to_chunk_block_tuple(pos);
    Chunk* chunk = world_get_or_create_chunk(world, tuple.cc);
    if (chunk == NULL)
        return NULL;
    
    return chunk_get_block(chunk, tuple.bc);
}

void __process_tick_events();

void world_tick(World* world)
{
    LOG_INFO("something tick world");
    __process_tick_events();
    // update chunk simulations
    // for (CHUNK IN CHUNKS)
}