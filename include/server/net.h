#pragma once
#include <memory>
#include <vector>
#include <prelude.h>
#include <enet.h>
#include <gdfe/os/thread.h>

struct NetClientManager {
    ENetHost* host;
    std::vector<ENetPeer> peers;

    GDF_Thread recv_thread;

    std::vector<std::unique_ptr<Services::Events::NetEvent>> incoming_queue;
    GDF_Mutex incoming_mutex;

    std::vector<std::unique_ptr<Services::Events::NetEvent>> outgoing_queue;
    GDF_Mutex outgoing_mutex;

    std::atomic_bool io_active;

    u16 port;

    NetClientManager(u16 port, u16 max_clients);
    ~NetClientManager();
    void broadcast();

    // Dispatches all the incoming events locally
    void dispatch_incoming();
};

