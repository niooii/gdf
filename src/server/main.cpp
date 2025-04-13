#include <gdfe/gdfe.h>
#include <stdexcept>
#define ENET_IMPLEMENTATION
#include <server/server.h>

#include <prelude.h>
#include <gdfe/strutils.h>
#include <server/net.h>

#include "game/events/defs.h"

GDF_BOOL server_loop(const GDF_AppState* app_state, f64 delta_time, void* _state)
{
    WorldServer* server = (WorldServer*)_state;
    server->tick();

    Services::Events::flush();
    return GDF_TRUE;
}

int main(int argc, char** argv)
{
    WorldServerCreateInfo server_info = {

    };

    if (argc < 2)
    {
        // ran as standalone program
    }
    else
    {
        const char* semaphore_name = argv[1];
        server_info.global_semaphore_name = argv[1];
    }

    GDF_InitSubsystems();
    if (enet_initialize() != 0) {
        LOG_FATAL("An error occurred while initializing ENet");
    }
    // GDF_Process proc = GDF_CreateProcess("vkcube.exe", NULL, NULL, NULL);
    // GDF_TerminateProcess(proc);

    WorldServer server{server_info};

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

    Services::Events::reject_dispatch_if<ChunkUpdateEvent>(
        [](auto event) {
        return event.source == ProgramType::Client;
    });

    GDF_Run();
    enet_deinitialize();
}
