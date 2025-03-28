#pragma once
#include "enet.h"
#include <game/events.h>
#include <gdfe/os/thread.h>

struct ServerNetworkManager {
    ENetHost* host;
    std::vector<ENetPeer> peers;
    std::vector<EventBase> incoming_queue;
    std::vector<EventBase> dispatch_queue;
    u16 port;

    GDF_Mutex cont_listening_lock;
    bool continue_listening;

    ServerNetworkManager(u16 port, u16 max_clients);
    ~ServerNetworkManager();
    void broadcast();

    // Dispatches all the incoming events locally and then
    // dispatches the oens in the dispatch queue to the proper
    // clients.
    void dispatch_events();
};

