#pragma once
#include <memory>
#include <vector>
#include <prelude.h>
#include <enet.h>
#include <gdfe/os/thread.h>

using Services::Events::NetEvent;

struct ConnectedClient {
    ENetPeer* peer;
    ConnectedClient(ENetPeer* peer) : peer{peer}
    {
        outgoing_queue.reserve(32);
        outgoing_mutex = GDF_CreateMutex();
    };

    ~ConnectedClient()
    {
        GDF_DestroyMutex(outgoing_mutex);
    }

    FORCEINLINE void queue_send(std::unique_ptr<NetEvent> event)
    {
        GDF_LockMutex(outgoing_mutex);
        outgoing_queue.push_back(std::move(event));
        GDF_ReleaseMutex(outgoing_mutex);
    }

    std::string auth;
    std::string uuid;
    std::string name;

    std::vector<std::unique_ptr<NetEvent>> outgoing_queue;
    GDF_Mutex outgoing_mutex;
};

struct ServerNetManager {
    ENetHost* host;

    /// A map of a UUID string to a ConnectedClient ptr.
    ankerl::unordered_dense::map<std::string, ConnectedClient*> clients;

    GDF_Thread recv_thread;

    std::vector<std::unique_ptr<NetEvent>> incoming_queue;
    GDF_Mutex incoming_mutex;

    std::atomic_bool io_active;

    u16 port;

    ServerNetManager(u16 port, u16 max_clients);
    ~ServerNetManager();

    FORCEINLINE void send_to(const std::string& uuid, std::unique_ptr<NetEvent> event)
    {
        auto entry = clients.find(uuid);
        if (entry == clients.end())
        {
            LOG_ERR("Tried to send packet to nonexisting client.");
            return;
        }
        entry->second->queue_send(std::move(event));
    }

    FORCEINLINE void broadcast(std::unique_ptr<NetEvent> event)
    {
        GDF_LockMutex(broadcast_mutex);
        broadcast_queue.push_back(std::move(event));
        GDF_ReleaseMutex(broadcast_mutex);
    }

    // Dispatches all the incoming events locally
    FORCEINLINE void dispatch_incoming()
    {
        GDF_LockMutex(incoming_mutex);
        for (const auto& event : incoming_queue)
        {
            event->queue_dispatch();
        }

        incoming_queue.clear();
        GDF_ReleaseMutex(incoming_mutex);
    }

    std::vector<std::unique_ptr<NetEvent>> broadcast_queue;
    GDF_Mutex broadcast_mutex;
};

