#include <gdfe/gdfe.h>
#define ENET_IMPLEMENTATION
#include <enet.h>
#include <stdexcept>
#include <server/net.h>


GDF_BOOL server_loop(const GDF_AppState* app_state, f64 delta_time, void* _state)
{
    Server* server = (Server*)_state;
    return GDF_TRUE;
}

int main()
{
    Server* server;
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

    server = new Server{25566, 64};

    GDF_Run();
}