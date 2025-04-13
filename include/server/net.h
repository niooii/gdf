#pragma once
#include <memory>
#include <vector>
#include <prelude.h>
#include <enet.h>
#include <gdfe/os/thread.h>


struct NetworkManager {
    ENetHost* host;
    std::vector<ENetPeer> peers;

    GDF_Thread recv_thread;

    std::vector<std::unique_ptr<Services::Events::Event>> incoming_queue;
    GDF_Mutex incoming_mutex;

    std::vector<std::unique_ptr<Services::Events::Event>> outgoing_queue;
    GDF_Mutex outgoing_mutex;

    std::atomic_bool io_active;

    u16 port;

    NetworkManager(u16 port, u16 max_clients);
    ~NetworkManager();
    void broadcast();

    // Dispatches all the incoming events locally
    void dispatch_incoming();
};

