#pragma once
#include <gdfe/core.h>
#include <game/world.h>
#include <game/events/defs.h>

#include "net.h"

/*
 * Abstractions around interacting with the world from the client.
 * A lot of methods here will queue network events. The synchronization of
 * the client's world state and servers is handled here as well.
 */

// Represents a connection to a world server on the client
class ClientWorld {
    // This may be null if the connection is not finished
    World* world_ = NULL;
    ServerConnection server_con_;

    ecs::Entity main_player_;

    // Creates a humanoid action event packet based on the current frame's inputs
    std::unique_ptr<HumanoidActionEvent> make_action_packet();

public:
    ClientWorld(const char* host, u16 port)
        : server_con_{host, port} {
        // TODO! remove and replaec with event system,
        // deserializing world data and constructing it when ready
        world_ = new World{"awf"};
        main_player_ = world_->create_humanoid();

        auto test_event = std::make_unique<TestTextEvent>();
        test_event->message = "HELLO SERVER!";
    }

    ~ClientWorld() {
        delete world_;
    }

    FORCEINLINE World* world_ptr() { return world_; }

    void update(f32 dt);
};