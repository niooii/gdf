#pragma once
#include <memory>
#include <vector>
#include <prelude.h>
#include <enet.h>
#include <gdfe/os/thread.h>

using Net::Packet;

struct ConnectedClient {
    ConnectedClient(ENetPeer* peer) : peer{peer}
    {
        outgoing_queue.reserve(32);
        outgoing_mutex = GDF_CreateMutex();
    };

    ~ConnectedClient()
    {
        GDF_DestroyMutex(outgoing_mutex);
    }

    FORCEINLINE void queue_send(std::unique_ptr<Packet> event)
    {
        GDF_LockMutex(outgoing_mutex);
        outgoing_queue.push_back(std::move(event));
        GDF_ReleaseMutex(outgoing_mutex);
    }

    ENetPeer* peer;

    std::string auth;
    std::string uuid;
    std::string name;

    std::vector<std::unique_ptr<Packet>> outgoing_queue;
    GDF_Mutex outgoing_mutex;
};

struct ServerNetManager {
    ENetHost* host;

    /// A map of a UUID string to a shared_ptr<ConnectedClient>.
    ankerl::unordered_dense::map<std::string, std::shared_ptr<ConnectedClient>>
        clients;

    GDF_Mutex clients_mutex;

    /// This set stores the clients that have not yet sent any authentication
    /// information over, but have connected to the server.
    /// This will only be accessed from one thread, so no locks are needed.
    ankerl::unordered_dense::map<ConnectedClient*, std::shared_ptr<ConnectedClient>>
        pending_connections;

    GDF_Thread recv_thread;

    std::vector<std::unique_ptr<Packet>> incoming_queue;
    GDF_Mutex incoming_mutex;

    std::atomic_bool io_active;

    u16 port;

    ServerNetManager(u16 port, u16 max_clients);
    ~ServerNetManager();

    FORCEINLINE void send_to(const std::string& uuid, std::unique_ptr<Packet> event)
    {
        GDF_LockMutex(clients_mutex);
        const auto entry = clients.find(uuid);
        if (entry == clients.end())
        {
            LOG_ERR("Tried to send packet to nonexisting client.");
        }
        else
        {
            entry->second->queue_send(std::move(event));
        }
        GDF_ReleaseMutex(clients_mutex);
    }

    FORCEINLINE void broadcast(std::unique_ptr<Packet> event)
    {
        GDF_LockMutex(broadcast_mutex);
        broadcast_queue.push_back(std::move(event));
        GDF_ReleaseMutex(broadcast_mutex);
    }

    // Dispatches all the incoming events locally
    FORCEINLINE void update()
    {
        GDF_LockMutex(incoming_mutex);
        for (const auto& event : incoming_queue)
        {
            event->queue_dispatch();
        }

        incoming_queue.clear();
        GDF_ReleaseMutex(incoming_mutex);
    }

    std::vector<std::unique_ptr<Packet>> broadcast_queue;
    GDF_Mutex broadcast_mutex;
};

