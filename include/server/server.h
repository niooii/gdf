#pragma once
#include <gdfe/os/thread.h>
#include <prelude.h>
#include <game/world.h>
#include <server/net.h>

// not a minecraft clone
#define GDF_SERVER_PORT 25566

// TODO! have a properties file and read from it with cereal
struct WorldServerCreateInfo {
    // This is the name of a global semaphore that will signal when the server should terminate
    // This is used to synchronize the client's local server and the actual client application
    const char* global_semaphore_name;
};

// TODO! world configuration
struct WorldServer {
    // This sempahore will be non-null if the server was started
    // on a player's local machine. If ran as a standalone program, this
    // will be NULL.
    GDF_Semaphore global_semaphore;
    NetworkManager net;
    World world;

    WorldServer(WorldServerCreateInfo& create_info)
        : net{GDF_SERVER_PORT, 64}, world{"daworld"}
    {
        if (create_info.global_semaphore_name)
        {
            global_semaphore = GDF_GetSemaphore(create_info.global_semaphore_name);
            LOG_FATAL("Could not acquire global semaphore.. this probably was not supposed to happen.");
            LOG_FATAL("Supplied sempahore name: %s", create_info.global_semaphore_name);
        }
    }

    void tick() {
        net.dispatch_incoming();
    }
};