#pragma once

#include <gdfe/core.h>
#include <game/world.h>
#include <gdfe/camera.h>

#include <gdfe/gdfe.h>

#include <client/graphics/renderer.h>

#include "net.h"

// TODO! replace with UIManager to push and pop screens or smthing
// typedef enum GDF_GAME_SCREEN {
//     GDF_GAME_SCREEN_MAIN_MENU,
//     GDF_GAME_SCREEN_IN_WORLD,
// } GDF_GAME_SCREEN;
//
// typedef enum GDF_GAME_SCREENTYPE {
//     GDF_GAME_SCREENTYPE_GUI_MENU,
//     GDF_GAME_SCREENTYPE_WORLD,
//     GDF_GAME_SCREENTYPE_WORLD_GUI_MENU,
// } GDF_GAME_SCREENTYPE;

struct ClientWorld;
struct App {
    // This will be NULL if the player is not in a world.
    ClientWorld* client_world = NULL;
    GameRenderer* renderer;

    // HumanoidEntity* main_player;
    // GDF_GAME_SCREEN current_screen;
    // GDF_GAME_SCREENTYPE current_screen_type;

    GDF_Camera main_camera;

    void join_world(const char* host, u16 port);
};

// The global app state.
extern App APP;

// Represents a connection to a world server on the client
struct ClientWorld {
    // This may be null if the connection is not finished
    World* world = NULL;
    ServerConnection server_con;
    ecs::Entity main_player;

    ClientWorld(const char* host, u16 port)
        : server_con{host, port} {
        // TODO! remove and replaec with event system,
        // deserializing world data and constructing it when ready
        world = new World{"awf"};
        APP.renderer->world_renderer.set_world(world);
    }

    ~ClientWorld() {
        delete world;
    }

    FORCEINLINE void update(f32 dt)
    {
        if (world)
            world->update(dt);
        server_con.dispatch_incoming();
        // TODO! remove
        // (but on the server we would update our state machines here, after dispatching incoming)
        auto view = world->registry().view<Components::MovementControl>();
        for(auto [entity, movement_ctl]: view.each())
        {
            movement_ctl.state_machine->update();
        }
    }
};


void app_init();
void app_destroy();
GDF_BOOL app_update(const GDF_AppState* app_state, f64 delta_time, void* state);

