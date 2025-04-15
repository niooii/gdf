#pragma once
#include <gdfe/os/thread.h>
#include <prelude.h>
#include <game/world.h>
#include <server/net.h>

// not a minecraft clone
#define GDF_SERVER_PORT 25566

// TODO! have a properties file and read from it with cereal
struct WorldServerCreateInfo {

};

// TODO! world configuration
class ServerWorld {
    NetworkManager net;
    World world;

public:

    ServerWorld(WorldServerCreateInfo& create_info)
        : net{GDF_SERVER_PORT, 64}, world{"daworld"}
    {

    }

    FORCEINLINE World* world_ptr() { return &world; }

    void tick() {
        net.dispatch_incoming();
    }
};