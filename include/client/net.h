#pragma once
#define ENET_DEBUG
#include <enet.h>
#include <memory>
#include <vector>
#include <gdfe/core.h>
#include <gdfe/os/thread.h>

struct EventBase;
struct ServerConnection {
    ENetHost* client;
    ENetPeer* peer;
    std::vector<std::unique_ptr<EventBase>> incoming_queue;
    // std::vector<std::unique_ptr<EventBase>> dispatch_queue;

    const char* addr;
    u16 port;

    GDF_Mutex cont_listening_lock;
    bool continue_listening;

    ServerConnection(const char* addr, u16 port);
    ~ServerConnection();

    void send(std::unique_ptr<EventBase> unique_ptr);

    void dispatch_incoming();
};