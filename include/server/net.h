#pragma once
#include "enet.h"
#include <game/events.h>

class Server {
    ENetHost* host;
    std::vector<ENetPeer> peers;
    std::vector<EventBase> incoming_queue;
    std::vector<EventBase> dispatch_queue;

public:
    Server(u16 port, u16 max_clients);
    void broadcast();
};

