#include <game/game.h>
#include <gdfe/core.h>
#include <game/server.h>
#include <gdfe/collections/hashmap.h>
#include <graphics/renderer.h>

#include "../gdfe/include/gdfe/gdfe.h"
#include "gdfe/input.h"

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

GDF_BOOL on_frame(const GDF_AppState* app_state, f64 delta_time, void* state) {
    if (GDF_IsKeyDown(GDF_KEYCODE_D)) {
        LOG_DEBUG("YAY!!");
    }

    return GDF_TRUE;
}

int main()
{
    Cube3State* game = game_init();
    GDF_InitInfo init = {
        .callbacks = {
            .on_frame = game_update,
            .on_frame_state = game,
            .render_callbacks = {
                .on_render_init = renderer_init,
                .on_render_init_state = game,
                .on_render_destroy = renderer_destroy,
                .on_render_destroy_state = game,
                .on_render = renderer_draw,
                .on_render_state = game
            }
        },
        .config = {
            .fps_cap = 0,
        }
    };
    GDF_AppState* app_state = GDF_Init(init);
    if (!app_state)
        return 1;

    game_init_world(game);

    GDF_RendererSetActiveCamera(app_state->renderer, game->main_camera);

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
