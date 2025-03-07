#include <game/game.h>
#include <os/sysinfo.h>
#include <app.h>
#include <asserts.h>
#include <subsystems.h>
#include <os/thread.h>
#include <game/server.h>
#include <collections/hashmap.h>

#include "gdfe.h"

unsigned long server_thread_wrapper(void* args)
{
    GDF_InitThreadLogging("Server");
    WorldServer ctx;
    WorldServerStartInfo start_info = {
        .max_clients = 20,
    };
    world_server_init(&start_info, &ctx);
    world_server_run(&ctx);
    return 0;
}

bool on_frame(const GDF_AppState* app_state, f64 delta_time, void* state) {
    if (GDF_IsKeyDown(GDF_KEYCODE_D)) {
        LOG_DEBUG("YAY!!");
    }



    return true;
}

int main()
{
    GDF_InitInfo init = {
        .callbacks = {
            .on_frame = on_frame
        }
    };
    if (!GDF_Init(init))
        return 1;

    f64 time_ran_for = GDF_Run();
    if (time_ran_for != -1)
    {
        LOG_INFO("App has been runinng for %lf seconds... Time to rest!", time_ran_for);
    }
    else
    {
        LOG_ERR("yikes....\n");
    }
    logging_flush_buffer();
    return 0;
}
