#pragma once
#include <memory>
#include <vector>

#include <enet.h>
#include <gdfe/os/thread.h>

struct EventBase;
struct ServerNetworkManager {
    ENetHost* host;
    std::vector<ENetPeer> peers;

    GDF_Thread recv_thread;
    std::vector<std::unique_ptr<EventBase>> incoming_queue;
    std::atomic_bool continue_listening;

    // std::vector<std::unique_ptr<EventBase>> dispatch_queue;

    u16 port;

    ServerNetworkManager(u16 port, u16 max_clients);
    ~ServerNetworkManager();
    void broadcast();

    // Dispatches all the incoming events locally and then
    // dispatches the oens in the dispatch queue to the proper
    // clients.
    void dispatch_incoming();
};

