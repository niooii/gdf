#pragma once
#include <game/events/defs.h>
#include <game/net.h>
#include <game/world.h>
#include <gdfe/prelude.h>
#include "net.h"

/*
 * Abstractions around interacting with the world from the client.
 * A lot of methods here will queue network events. The synchronization of
 * the client's world state and servers is handled here as well.
 */

// Represents a connection to a world server on the client
class ClientWorld {
    // This may be null if the connection is not finished
    World*           world_ = NULL;
    ServerConnection server_con_;

    ecs::Entity main_player_;

    // Creates a humanoid action event packet based on the current frame's inputs
    std::unique_ptr<HumanoidStateChangeEvent> make_action_packet();

public:
    ClientWorld(const char* host, u16 port) : server_con_{ host, port }
    {
        // TODO! remove and replaec with event system,
        // deserializing world data and constructing it when ready
        world_       = new World{ "awf" };
        main_player_ = world_->create_humanoid();

        auto client_info           = Services::Events::create_event<ClientConnectionEvent>();
        client_info->auth          = "TESTABABABAB";
        client_info->uuid          = "TESTUUID ABABABA";
        client_info->connect_event = true;
        server_con_.send(std::move(client_info));

        auto test_event     = Services::Events::create_event<TestMsgEvent>();
        test_event->message = "HELLO SERVER!";
        server_con_.send(std::move(test_event));
    }

    ~ClientWorld() { delete world_; }

    FORCEINLINE World* world_ptr() { return world_; }

    void update(f32 dt);
};
