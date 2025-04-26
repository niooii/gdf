#pragma once

#include <ser20/ser20.hpp>
#include <ser20/types/polymorphic.hpp>
#include <serde.h>
#include <vector>
#include <game/types.h>
#include <prelude.h>

/*
 * This file contains all the network specific components of the game,
 * including specific events (packets) meant for use over the net only
 */

/// To send a message to the server for testing purposes.
DECL_NET_EVENT(TestMsgEvent)
{
    std::string message;
    SERIALIZE_EVENT_FIELDS(message)
};

/// Contains everything the server needs to know about a client upon connection.
DECL_NET_EVENT(ClientConnectionEvent)
{
    /// TODO! This is unused for now. For development purposes, the only field that matters is
    /// the UUID. Change before production.
    std::string auth;
    /// The UUID of the player. Do not rely on this field, it will be removed once proper authentication
    /// is implemented.
    std::string uuid;
    SERIALIZE_EVENT_FIELDS(auth, uuid)
};

DECL_NET_EVENT(ClientDisconnectEvent)
{
    /// TODO! This is unused for now. For development purposes, the only field that matters is
    /// the UUID. Change before production.
    std::string auth;
    /// The UUID of the player who has disconnected
    std::string uuid;
};

