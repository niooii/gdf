#include <client/graphics/renderer.h>
#include <game/entity/entity.h>
#include <game/events/defs.h>
#include <game/world.h>
#include <gdfe/event.h>
#include <gdfe/profiler.h>
#include <prelude.h>

// TODO! make customizable
World::World(WorldCreateInfo& create_info)
{
    chunks_.reserve(128);
    SimulationCreateInfo physics_info = { .gravity = vec3_mul_scalar(vec3_new(0, -1, 0), 20),
        .gravity_active                            = GDF_TRUE,
        .air_drag                                  = 3.f,
        .ground_drag                               = 12.f,
        .terminal_velocity                         = -50.f,
        .entity_registry_p                         = &registry_ };

    chunk_sim_dist_  = 8;
    chunk_view_dist_ = 8;

    physics_       = new PhysicsSimulation(physics_info, this);
    ticks_per_sec_ = 20;

    upd_stopwatch_ = GDF_StopwatchCreate();

    auto chunk_load_event    = Services::Events::create_event<ChunkLoadEvent>();
    chunk_load_event->source = ProgramType::Client;

    // Create chunks
    for (i32 chunk_x = -1; chunk_x <= 1; chunk_x++)
    {
        for (i32 chunk_y = -1; chunk_y < 1; chunk_y++)
        {
            for (i32 chunk_z = -1; chunk_z <= 1; chunk_z++)
            {
                ivec3 cc    = { .x = chunk_x, .y = chunk_y, .z = chunk_z };
                chunks_[cc] = new Chunk();
                generator_.gen_chunk(cc, *chunks_[cc]);
                chunk_load_event->loaded_chunks.push_back(ChunkLoadInfo{ cc });
            }
        }
    }

    Services::Events::queue_dispatch(*chunk_load_event);
}

// TODO! actually implement loading and saving
World::World(const char* folder_path)
{
    chunks_.reserve(128);
    SimulationCreateInfo physics_info = { .gravity = vec3_mul_scalar(vec3_new(0, -1, 0), 20),
        .gravity_active                            = GDF_TRUE,
        .air_drag                                  = 3.f,
        .ground_drag                               = 12.f,
        .terminal_velocity                         = -50.f,
        .entity_registry_p                         = &registry_ };

    chunk_sim_dist_  = 8;
    chunk_view_dist_ = 8;

    physics_       = new PhysicsSimulation(physics_info, this);
    ticks_per_sec_ = 20;

    upd_stopwatch_ = GDF_StopwatchCreate();

    auto chunk_load_event = Services::Events::create_event<ChunkLoadEvent>();

    // Create chunks
    for (i32 chunk_x = -1; chunk_x <= 1; chunk_x++)
    {
        for (i32 chunk_y = -1; chunk_y < 1; chunk_y++)
        {
            for (i32 chunk_z = -1; chunk_z <= 1; chunk_z++)
            {
                ivec3 cc    = { .x = chunk_x, .y = chunk_y, .z = chunk_z };
                chunks_[cc] = new Chunk();
                generator_.gen_chunk(cc, *chunks_[cc]);
                chunk_load_event->loaded_chunks.emplace_back(cc);
            }
        }
    }

    Services::Events::queue_dispatch(*chunk_load_event);
}

World::~World()
{
    for (auto& entry : chunks_)
    {
        delete entry.second;
    }
}

void World::update(f64 dt)
{
    /* Update the world simulation */
    physics_->update(dt);
    for (auto& hum : humanoids_)
    {
        hum.update();
    }
}

Chunk* World::get_chunk(ivec3 chunk_coord) const
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

ecs::Entity World::create_humanoid()
{
    auto entity = registry_.create();
    registry_.emplace<Components::Health>(entity, 100.f);
    registry_.emplace<Components::Velocity>(entity, vec3_zero());
    registry_.emplace<Components::Rotation>(entity);
    AxisAlignedBoundingBox aabb = { vec3_new(-0.375, 0, -0.375), vec3_new(0.375, 1.8, 0.375) };
    aabb_translate(&aabb, vec3_new(1, 10, 1));
    registry_.emplace<Components::AabbCollider>(entity, aabb, false);

    humanoids_.emplace_back(entity);
    return entity;
}

// WILL NOT create a new chunk if it doesn't exist.
Block* World::get_block(vec3 pos) const
{
    auto [cc, bc] = world_pos_to_chunk_block_tuple(pos);
    Chunk* chunk  = this->get_chunk(cc);
    if (!chunk)
        return nullptr;

    return chunk->get_block(bc);
}

Block* World::set_block(BlockCreateInfo& create_info)
{
    ChunkBlockPosTuple info = world_pos_to_chunk_block_tuple(create_info.world_pos);
    Chunk*             c    = this->get_or_create_chunk(info.cc);
    Block*             b    = c->set_block(create_info.type, info.bc);

    ChunkUpdateEvent update{};
    update.chunk_coord = info.cc;
    Services::Events::dispatch(update);

    // TODO! remove this please find a way to fit this inside the chunk functions directly.
    // or better yet through the use of actual structured event data. PLEASE!!
    // chunk update queue remesh stuff
    u8vec3 bc = info.bc;

    // GDF_EventFire(GDF_EVENT_CHUNK_UPDATE, c, (GDF_EventContext){});

    return b;
}

// Gets the blocks that is touching an AABB.
// Modifies the result_arr with the found blocks, and returns the amount of blocks found
const u32 World::get_blocks_touching(
    AxisAlignedBoundingBox* aabb, BlockTouchingResult* result_arr, u32 result_arr_size) const
{
    f32    min_x = FLOOR(aabb->min.x);
    f32    min_y = FLOOR(aabb->min.y);
    f32    min_z = FLOOR(aabb->min.z);
    f32    max_x = FLOOR(aabb->max.x);
    f32    max_y = FLOOR(aabb->max.y);
    f32    max_z = aabb->max.z;
    u32    i     = 0;
    Block* cb    = nullptr;
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
                res->block               = cb;
                res->box                 = (AxisAlignedBoundingBox){ .min = vec3_new(x, y, z),
                                    .max                                  = vec3_new(x + 1, y + 1, z + 1) };

                if (i >= result_arr_size)
                    return i;
            }
        }
    }

    return i;
}

void world_tick(World* world)
{
    LOG_INFO("something tick world");
    // update chunk simulations
    // for (CHUNK IN CHUNKS)
}
