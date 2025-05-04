#pragma once
#include <gdfe/gdfe.h>
#include <server/world.h>

struct Server {
    // This is the name of a global semaphore that will signal when the server should terminate
    // This is used to synchronize the client's local server and the actual client application
    const char* global_semaphore_name = NULL;

    // This sempahore will be non-null if the server was started
    // on a player's local machine. If ran as a standalone program, this
    // will be NULL.
    GDF_Semaphore global_semaphore;

    ServerNetManager* net;

    ServerWorld* server_world;
};

// The global server state
extern Server SERVER;

void     server_init();
void     server_destroy();
GDF_BOOL server_update(const GDF_AppState* app_state, f64 delta_time, void* state);
