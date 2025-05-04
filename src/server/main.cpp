#include <gdfe/gdfe.h>
#include <stdexcept>
#define ENET_IMPLEMENTATION
#include <server/server.h>

#include <gdfe/strutils.h>
#include <prelude.h>
#include <server/net.h>

#include "game/events/defs.h"

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
        const char* semaphore_name   = argv[1];
        SERVER.global_semaphore_name = semaphore_name;
    }

    GDF_InitSubsystems();
    if (enet_initialize() != 0)
    {
        LOG_FATAL("An error occurred while initializing ENet");
    }

    // test process api
    // GDF_Process proc = GDF_CreateProcess("vkcube.exe", NULL, NULL, NULL);
    // GDF_TerminateProcess(proc);

    server_init();

    GDF_InitInfo info = { .config = { .updates_per_sec = 50, .disable_video = GDF_TRUE },
        .callbacks                = { .on_loop = server_update, .on_loop_state = &SERVER } };
    GDF_Init(info);

    GDF_Run();
    enet_deinitialize();
}
