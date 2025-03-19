#include <game/world.h>
#include <events.h>
#include <gdfe/event.h>
#include <gdfe/profiler.h>
#include <client/graphics/renderer.h>

u32 chunk_hash(const u8* data, u32 len) {

}
//
// static GDF_BOOL __on_chunk_load(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx)
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
//     return GDF_FALSE;
// }
//
// static GDF_BOOL __on_chunk_unload(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx)
// {
//     LOG_DEBUG("CHUNK UNLOAD...");
//     World* world = (World*)listener_instance;
//
//     Chunk* chunk = (Chunk*)sender;
//     renderer_queue_chunk_remesh(chunk->cc);
//
//     GDF_ASSERT(renderer_remove_chunk(chunk->cc));
//
//     return GDF_FALSE;
// }
//
// // TODO!
// static GDF_BOOL __on_chunk_update(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx)
// {
//     LOG_DEBUG("CHUNK UPDATE");
//     World* world = (World*)listener_instance;
//     Chunk* chunk = (Chunk*)sender;
//
//     renderer_queue_chunk_remesh(chunk->cc);
//
//     return GDF_FALSE;
// }

static GDF_BOOL __on_block_touch(u16 event_code, void* sender, void* listener_instance, GDF_EventContext ctx)
{
    World* world = (World*)listener_instance;
    Block* block = (Block*)sender;
    Entity* entity = (Entity*)(ctx.data.u64[0]);
    if (entity->type != ENTITY_TYPE_HUMANOID)
        return GDF_FALSE;

    // HumanoidEntity* hum = entity->parent;

    return GDF_FALSE;
}

// TODO! make customizable
World::World()
    : generator_{this}
{
    chunks_.reserve(128);
    humanoids_.reserve(128);
    PhysicsCreateInfo physics_info = {
        .gravity = {0, -20, 0},
        .gravity_active = GDF_TRUE,
        .air_drag = 3.f,
        .ground_drag = 12.f,
        .terminal_velocity = -50.f
    };

    chunk_sim_dist_ = 8;
    chunk_view_dist_ = 8;

    // TODO! make physics class
    physics_ = physics_init(physics_info);
    ticks_per_sec_ = 20;

    upd_stopwatch_ = GDF_StopwatchCreate();

    // Create chunks
    for (i32 chunk_x = -4; chunk_x <= 4; chunk_x++)
    {
        for (i32 chunk_y = -2; chunk_y < 2; chunk_y++)
        {
            for (i32 chunk_z = -4; chunk_z <= 4; chunk_z++)
            {
                ivec3 cc = {
                    .x = chunk_x,
                    .y = chunk_y,
                    .z = chunk_z
                };
                chunks_[cc] = new Chunk();
                generator_.gen_chunk(cc, *chunks_[cc]);
            }
        }
    }

    // for (u32 i = 0; i < num_chunks; i++)
    // {
    //     // renderer_register_chunk(created_chunks[i]);
    // }

    // TODO! make events a SET of listeners not a list no problem
    // GDF_ASSERT(GDF_EventRegister(GDF_EVENT_BLOCK_TOUCHED, out_world, __on_block_touch));
    // GDF_ASSERT(GDF_EventRegister(GDF_EVENT_CHUNK_LOAD, out_world, __on_chunk_load));
    // GDF_ASSERT(GDF_EventRegister(GDF_EVENT_CHUNK_UNLOAD, out_world, __on_chunk_unload));
    // GDF_ASSERT(GDF_EventRegister(GDF_EVENT_CHUNK_UPDATE, out_world, __on_chunk_update));
}

World::~World()
{
    for (auto& entry : chunks_)
    {
        delete entry.second;
    }
    for (auto hum : humanoids_)
    {
        delete hum;
    }
}

void World::update(f64 dt)
{
    u32 num_humanoids = humanoids_.size();
    for (u32 i = 0; i < num_humanoids; i++)
        humanoid_entity_update(humanoids_[i]);
    physics_update(physics_, this, dt);
}

Chunk* World::get_chunk(ivec3 chunk_coord)
{
    auto it = chunks_.find(chunk_coord);
    if (it == chunks_.end())
    {
        // LOG_ERR("NO CHUNK");
        return nullptr;
    }

    return it->second;
}

Chunk* World::get_or_create_chunk(ivec3 chunk_coord)
{
    auto it = chunks_.find(chunk_coord);
    if (it == chunks_.end())
    {
        chunks_[chunk_coord] = new Chunk();
        generator_.gen_chunk(chunk_coord, *chunks_[chunk_coord]);
        return chunks_[chunk_coord];
    }

    return it->second;
}

HumanoidEntity* World::create_humanoid()
{
    auto* hum = new HumanoidEntity();
    humanoids_.emplace_back(hum);
    hum->base.type = ENTITY_TYPE_HUMANOID;
    hum->base.parent = hum;

    return hum;
}

// Will not create a new chunk if it doesn't exist.
Block* World::get_block(vec3 pos)
{
    auto [cc, bc] = world_pos_to_chunk_block_tuple(pos);
    Chunk* chunk = this->get_chunk(cc);
    if (!chunk)
        return nullptr;

    return chunk->get_block(bc);
}

// Will create a new chunk if it doesn't exist.
Block* World::get_block_gen_chunk(vec3 pos)
{
    auto [cc, bc] = world_pos_to_chunk_block_tuple(pos);
    Chunk* chunk = this->get_or_create_chunk(cc);
    if (!chunk)
        return nullptr;

    return chunk->get_block(bc);
}

Block* World::set_block(BlockCreateInfo& create_info)
{
    ChunkBlockPosTuple info = world_pos_to_chunk_block_tuple(create_info.world_pos);
    Chunk* c = this->get_or_create_chunk(info.cc);
    Block* b = c->set_block(create_info.type, info.bc);

    // TODO! remove this please find a way to fit this inside the chunk functions directly.
    // or better yet through the use of actual structured event data. PLEASE!!
    // chunk update queue remesh stuff
    RelBlockCoord bc = info.bc;

    // GDF_EventFire(GDF_EVENT_CHUNK_UPDATE, c, (GDF_EventContext){});

    return b;
}

void World::destroy_block(vec3 pos, Block* destroyed)
{
    ChunkBlockPosTuple info = world_pos_to_chunk_block_tuple(pos);
    Chunk* c = this->get_or_create_chunk(info.cc);
    c->destroy_block(info.bc, nullptr);

    // TODO! remove this please find a way to fit this inside the chunk functions directly.
    // or better yet through the use of actual structured event data. PLEASE!!
    // OR MAYBE JUST MAEK THE EVENT SYSTEM TO POOL CHANGES . YES.,

    // GDF_EventFire(GDF_EVENT_CHUNK_UPDATE, c, (GDF_EventContext){});
}

// Gets the blocks that is touching an AABB.
// Modifies the result_arr with the found blocks, and returns the amount of blocks found
u32 World::get_blocks_touching(
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
    Block* cb = nullptr;
    for (f32 x = min_x; x <= max_x; x++)
    {
        for (f32 y = min_y; y <= max_y; y++)
        {
            for (f32 z = min_z; z <= max_z; z++)
            {
                cb = this->get_block(vec3_new(x, y, z));
                if (!cb)
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

void __process_tick_events();

void world_tick(World* world)
{
    LOG_INFO("something tick world");
    __process_tick_events();
    // update chunk simulations
    // for (CHUNK IN CHUNKS)
}