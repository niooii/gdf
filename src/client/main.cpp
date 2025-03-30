#include <game/game.h>
#include <gdfe/core.h>
#include <client/graphics/renderer.h>
#include <game/prelude.h>

#define ENET_DEBUG
#define ENET_IMPLEMENTATION
#include <client/net.h>
#include <game/events/structs.h>
#include "gdfe/input.h"

GDF_BOOL on_frame(const GDF_AppState* app_state, f64 delta_time, void* state) {
    if (GDF_IsKeyDown(GDF_KEYCODE_D)) {
        LOG_DEBUG("YAY!!");
    }

    Cube3State* game = (Cube3State*)state;
    game_update(app_state, delta_time, state);

    return GDF_TRUE;
}

int main()
{
    Cube3State* game = game_init();
    GDF_InitInfo init = {
        .callbacks = {
            .on_loop = on_frame,
            .on_loop_state = game,
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
            .max_updates_per_sec = 0,
        }
    };
    GDF_AppState* app_state = GDF_Init(init);
    if (!app_state)
        return 1;

    ServerConnection connection{"127.0.0.1", 25566};
    connection.send(std::make_unique<TestTextEvent>());

    GDF_ThreadSleep(1000);
    return 0;

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
