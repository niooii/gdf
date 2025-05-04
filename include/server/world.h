#pragma once
#include <game/world.h>
#include <gdfe/os/thread.h>
#include <prelude.h>
#include <server/net.h>

// not a minecraft clone
#define GDF_SERVER_PORT 25566

// TODO! have a properties file and read from it with cereal
struct WorldServerCreateInfo {};

struct Client {
    std::vector<HumanoidStateChangeEvent> queued_inputs;
};

// TODO! world configuration
class ServerWorld {
    ServerNetManager* net_;
    World             world_;

    std::vector<std::unique_ptr<Services::Events::Subscription>> subscriptions_;
    // this should be stored per each client, probably.
    std::vector<HumanoidStateChangeEvent> clients;

public:
    ServerWorld(ServerNetManager* net) : net_{ net }, world_{ "daworld" }
    {
        // subscriptions_.emplace_back(
        //     // Services::Events::subscribe<>([&wo])
        // );
    }

    ~ServerWorld() {}

    FORCEINLINE World* world_ptr() { return &world_; }

    FORCEINLINE void update(f32 dt)
    {
        auto& humanoids = world_.simulated_humanoids();
        for (auto& humanoid : humanoids)
            humanoid.reset_accumulator();

        // TODO! humanoid ai stuff here + handled queued player inputs

        // then tick the simulation
        world_.update(dt);

        for (auto& humanoid : humanoids)
        {
            HumanoidStateChangeEvent accumulated = humanoid.accumulated_actions();
            // send here or something idk
        }
    }
};
