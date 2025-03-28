#include <gdfe/gdfe.h>

struct ServerState {

};

GDF_BOOL server_loop(const GDF_AppState* app_state, f64 delta_time, void* _state)
{
    LOG_INFO("type shi");
    ServerState* state = (ServerState*)_state;
    return GDF_TRUE;
}

int main()
{
    ServerState server_state = {

    };

    GDF_InitInfo info = {
        .config = {
            .max_updates_per_sec = 20,
            .disable_video = GDF_TRUE
        },
        .callbacks = {
            .on_loop = server_loop,
            .on_loop_state = &server_state
        }
    };
    GDF_Init(info);

    GDF_Run();
}