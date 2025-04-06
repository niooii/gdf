#include <gdfe/gdfe.h>
#include <stdexcept>
#include <server/server.h>
#ifndef GDF_CLIENT_BUILD
#define ENET_IMPLEMENTATION
#endif

#include <gdfe/strutils.h>
#include <server/net.h>

GDF_BOOL server_loop(const GDF_AppState* app_state, f64 delta_time, void* _state)
{
    ServerNetworkManager* server = (ServerNetworkManager*)_state;
    server->dispatch_incoming();
    return GDF_TRUE;
}

#include <gdfe/collections/list.h>
#ifndef GDF_CLIENT_BUILD
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        // ran as standalone program
    }
    else
    {
        const char* semaphore_name = argv[1];
    }

    GDF_InitSubsystems();

    ServerNetworkManager* server = new ServerNetworkManager{GDF_SERVER_PORT, 64};

    GDF_InitInfo info = {
        .config = {
            .max_updates_per_sec = 20,
            .disable_video = GDF_TRUE
        },
        .callbacks = {
            .on_loop = server_loop,
            .on_loop_state = server
        }
    };
    GDF_Init(info);

    GDF_Run();
}
#endif