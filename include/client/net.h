#pragma once
#include <atomic>
#include <enet.h>
#include <gdfe/os/thread.h>
#include <gdfe/prelude.h>
#include <memory>
#include <prelude.h>
#include <vector>

struct ServerConnection {
    ENetHost* client;
    ENetPeer* peer;

    GDF_Thread recv_thread;

    std::vector<std::unique_ptr<Net::Packet>> incoming_queue;
    GDF_Mutex                                 incoming_mutex;

    std::vector<std::unique_ptr<Net::Packet>> outgoing_queue;
    GDF_Mutex                                 outgoing_mutex;

    std::atomic_bool io_active;

    const char* addr;
    u16         port;

     ServerConnection(const char* addr, u16 port);
    ~ServerConnection();

    void send(std::unique_ptr<Net::Packet> unique_ptr);

    void dispatch_incoming();
};
