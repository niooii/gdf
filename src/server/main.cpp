#include <gdfe/gdfe.h>
#include <enet.h>
#include <stdexcept>

struct Server {
    Server(u16 port, u16 max_clients)
    {
        ENetAddress addr = {
            .port = port,
            .host = ENET_HOST_ANY
        };

        if (enet_initialize() != 0) {
            throw std::runtime_error("An error occurred while initializing ENet.\n");
        }

        ENetHost* host = enet_host_create(
            &addr,
            max_clients,
            2,
            0,
            0
        );

        if (host == NULL)
        {
            throw std::runtime_error("An error occurred while creating the server host.\n");
        }

        LOG_INFO("Listening on port %d...", port);
    }
};

GDF_BOOL server_loop(const GDF_AppState* app_state, f64 delta_time, void* _state)
{
    LOG_INFO("type shi");
    Server* server = (Server*)_state;
    return GDF_TRUE;
}

int main()
{
    Server server{25566, 64};

    GDF_InitInfo info = {
        .config = {
            .max_updates_per_sec = 20,
            .disable_video = GDF_TRUE
        },
        .callbacks = {
            .on_loop = server_loop,
            .on_loop_state = &server
        }
    };
    GDF_Init(info);

    GDF_Run();
}