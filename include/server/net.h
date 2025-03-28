#pragma once

class Server {
ENetHost* host;
std::vector<ENetPeer> peers;
std::vector<Event> incoming_queue   ;
std::vector<Event> dispatch_queue;

public:
    Server(u16 port, u16 max_clients);
    void broadcast();
}

