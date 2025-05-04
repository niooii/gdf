#include <gdfe/prelude.h>
#include <client/graphics/renderer.h>
#include <prelude.h>

#define ENET_IMPLEMENTATION
#include <client/app.h>
#include <server/net.h>
#include <server/server.h>

int main()
{
    GDF_InitSubsystems();
    if (enet_initialize() != 0) {
        LOG_FATAL("An error occurred while initializing ENet");
    }

    app_init();
    GDF_InitInfo init = {
        .callbacks = {
            .on_loop = app_update,
            .on_loop_state = &APP,
            .render_callbacks = {
                .on_render_init = renderer_init,
                .on_render_init_state = &APP,
                .on_render_destroy = renderer_destroy,
                .on_render_destroy_state = &APP,
                .on_render = renderer_draw,
                .on_render_state = &APP
            }
        },
        .config = {
            .updates_per_sec = 0,
        }
    };

    const char* test_buffer = "awfwfawfwa";
    GDF_StorageWrite("test", test_buffer, strlen(test_buffer));

    GDF_AppState* app_state = GDF_Init(init);
    if (!app_state)
        return 1;

    char server_path[300];
    GDF_Memzero(server_path, sizeof(server_path));
    snprintf(server_path, sizeof(server_path), "%s\\server.exe", GDF_GetExecutablePath());

    GDF_Process server_proc = GDF_CreateProcess(
        server_path,
        NULL,
        NULL,
        NULL
    );

    GDF_ThreadSleep(500);

    APP.join_world("127.0.0.1", GDF_SERVER_PORT);

    GDF_RendererSetActiveCamera(app_state->renderer, APP.main_camera);

    LOG_DEBUG("running path: %s", GDF_GetExecutablePath());
    f64 time_ran_for = GDF_Run();
    if (time_ran_for != -1)
    {
        LOG_INFO("App has been runinng for %lf seconds... Time to rest!", time_ran_for);
    }
    else
    {
        LOG_ERR("yikes....\n");
    }
    GDF_FlushLogBuffer();
    enet_deinitialize();
    return 0;
}
