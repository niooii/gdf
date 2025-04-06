#include <gdfe/gdfe.h>
#include <stdexcept>
#include <server/server.h>
#ifndef GDF_CLIENT_BUILD
#define ENET_IMPLEMENTATION
#endif

#include <server/net.h>

GDF_BOOL server_loop(const GDF_AppState* app_state, f64 delta_time, void* _state)
{
    ServerNetworkManager* server = (ServerNetworkManager*)_state;
    server->dispatch_incoming();
    return GDF_TRUE;
}

#ifndef GDF_CLIENT_BUILD
int main()
{
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