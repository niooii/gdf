#pragma once
#define ENET_DEBUG
#include <enet.h>
#include <memory>
#include <vector>
#include <gdfe/core.h>
#include <gdfe/os/thread.h>
#include <atomic>

struct EventBase;
struct ServerConnection {
    ENetHost* client;
    ENetPeer* peer;

    GDF_Thread recv_thread;

    std::vector<std::unique_ptr<EventBase>> incoming_queue;
    GDF_Mutex incoming_mutex;

    std::vector<std::unique_ptr<EventBase>> outgoing_queue;
    GDF_Mutex outgoing_mutex;

    std::atomic_bool io_active;

    // std::vector<std::unique_ptr<EventBase>> dispatch_queue;

    const char* addr;
    u16 port;

    ServerConnection(const char* addr, u16 port);
    ~ServerConnection();

    void send(std::unique_ptr<EventBase> unique_ptr);

    void dispatch_incoming();
};