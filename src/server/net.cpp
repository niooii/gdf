#include <game/events/defs.h>
#include <game/net.h>
#include <gdfe/os/thread.h>
#include <prelude.h>
#include <server/net.h>

using namespace Services;
static unsigned long io_thread(void* args)
{
    ServerNetManager* server = (ServerNetManager*)args;
    GDF_InitThreadLogging("Server:Net");

    const GDF_Stopwatch throttle_timer = GDF_StopwatchCreate();
    // Make loop happen at a fixed rate of 2ms per iteration
    constexpr f64 throttle_secs = 0.002;

    LOG_INFO("Listening on port %d", server->port);
    ENetEvent event;
    do
    {
        GDF_StopwatchReset(throttle_timer);
        // TODO! this is not good. probably batch serialize all events at once, and then
        // acquire the mutex for a short amount of time, although this would mean
        // separating the queued events into a global queue not owned by the ConnectedClient.
        // Look out if the server starts falling behind, will prob become a bottleneck
        GDF_LockMutex(server->clients_mutex);
        for (auto& [uuid, client] : server->clients)
        {
            GDF_LockMutex(client->outgoing_mutex);
            for (const auto& outgoing : client->outgoing_queue)
            {
                std::string serialized{ Net::serialize(outgoing) };

                ENetPacket* packet = enet_packet_create(
                    serialized.c_str(), serialized.length() + 1, ENET_PACKET_FLAG_RELIABLE);

                enet_peer_send(client->peer, 0, packet);
            }
            client->outgoing_queue.clear();
            GDF_ReleaseMutex(client->outgoing_mutex);
        }
        GDF_ReleaseMutex(server->clients_mutex);

        GDF_LockMutex(server->broadcast_mutex);
        for (auto& outgoing : server->broadcast_queue)
        {
            std::string serialized{ Net::serialize(outgoing) };

            ENetPacket* packet = enet_packet_create(
                serialized.c_str(), serialized.length() + 1, ENET_PACKET_FLAG_RELIABLE);

            enet_host_broadcast(server->host, 0, packet);
        }
        server->broadcast_queue.clear();
        GDF_ReleaseMutex(server->broadcast_mutex);

        while (enet_host_service(server->host, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                {
                    LOG_DEBUG("A new client connected from %x:%u.\n", event.peer->address.host,
                        event.peer->address.port);
                    std::shared_ptr<ConnectedClient> shared_client(new ConnectedClient(event.peer));
                    // the shared pointer will only go out of scope once
                    // the client disconnects, so we will not ever
                    // deal with this client again. therefore, we will not
                    // ever access it after it becomes dangling.
                    event.peer->data                                 = shared_client.get();
                    server->pending_connections[shared_client.get()] = shared_client;
                }
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                {
                    auto* client = (ConnectedClient*)event.peer->data;
                    try
                    {
                        auto recv_event = Net::deserialize(
                            { (char*)event.packet->data, event.packet->dataLength });

                        if (recv_event->source != ProgramType::Client)
                        {
                            LOG_WARN("Rejected packet for invalid packet source.");
                            goto SN_DESTROY_PACKET;
                        }

                        if (UNLIKELY(recv_event->connect_event))
                        {
                            // check if the connected client is pending a connection event
                            auto pending_client = server->pending_connections.extract(client);

                            if (!pending_client.has_value())
                            {
                                LOG_ERR(
                                    "Client \"%s\" tried to connect twice.", client->uuid.c_str());
                                // TODO! handle more later
                                goto SN_DESTROY_PACKET;
                            }

                            std::shared_ptr<ConnectedClient> shared_client =
                                pending_client.value().second;

                            const auto* conn =
                                (dynamic_cast<ClientConnectionEvent*>(recv_event.get()));

                            if (LIKELY(conn))
                            {
                                client->auth = conn->auth;
                                // TODO! get uuid and name from some centralized server
                                // or something? idk. (oh but don't block this thread uh gg)
                                // At this point, the shared ptr should only be accessed
                                // in this thread, so there should be no harm in
                                // directly assigning these values without a lock
                                shared_client->uuid = conn->uuid;
                                shared_client->name = "NAMELESS... FORMLESS...";
                                GDF_LockMutex(server->clients_mutex);
                                server->clients[conn->uuid] = shared_client;
                                GDF_ReleaseMutex(server->clients_mutex);

                                LOG_DEBUG("Initialized connection information for %s!",
                                    conn->uuid.c_str());
                            }
                            else
                            {
                                LOG_ERR("Recieved malformed connection packet, treating like a "
                                        "regular packet..");
                            }
                        }

                        recv_event->data = (void*)client;

                        GDF_LockMutex(server->incoming_mutex);
                        server->incoming_queue.push_back(std::move(recv_event));
                        GDF_ReleaseMutex(server->incoming_mutex);
                    }
                    catch (const ser20::Exception& e)
                    {
                        LOG_ERR("Failed to deserialize a packet from %s", client->uuid.c_str());
                    }
                SN_DESTROY_PACKET:
                    enet_packet_destroy(event.packet);
                }
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                {
                    GDF_LockMutex(server->incoming_mutex);
                    auto* client = (ConnectedClient*)event.peer->data;

                    LOG_DEBUG("%s disconnected.\n", client->name.c_str());

                    auto dc  = Services::Events::create_event<ClientDisconnectEvent>();
                    dc->auth = client->auth;
                    dc->uuid = client->uuid;
                    server->incoming_queue.push_back(std::move(dc));

                    GDF_ReleaseMutex(server->incoming_mutex);

                    // remove the client from the map so outgoing messages are no longer sent,
                    // effectively removing it from this thread's concern.
                    GDF_LockMutex(server->clients_mutex);
                    server->clients.erase(client->uuid);
                    GDF_ReleaseMutex(server->clients_mutex);

                    // client will delete itself via shared pointer.
                }
                break;

            case ENET_EVENT_TYPE_NONE:
                break;

            default:
                LOG_ERR("BAD ENET EVENT.");
            }
        }
        if (const f64 overflow = GDF_StopwatchSleepUntil(throttle_timer, throttle_secs);
            overflow > 0)
            LOG_WARN("Server fell behind by %lf secs :(", overflow);
    }
    while (server->io_active);

    GDF_StopwatchDestroy(throttle_timer);

    return 0;
}

ServerNetManager::ServerNetManager(u16 port, u16 max_clients)
{
    ENetAddress addr = { .port = port, .host = ENET_HOST_ANY };

    clients.reserve(32);

    Events::subscribe<TestMsgEvent>(
        [](auto& event)
        {
            const ConnectedClient* client = (const ConnectedClient*)event.data;
            LOG_INFO("Words from %s: \"%s\"", client->name.c_str(), event.message.c_str());
        });

    io_active  = true;
    this->port = port;

    incoming_mutex  = GDF_CreateMutex();
    broadcast_mutex = GDF_CreateMutex();
    clients_mutex   = GDF_CreateMutex();

    if (enet_initialize() != 0)
    {
        LOG_FATAL("An error occurred while initializing ENet");
    }

    this->host = enet_host_create(&addr, max_clients, 2, 0, 0);

    if (this->host == NULL)
    {
        LOG_FATAL("An error occurred while creating the server host");
    }

    recv_thread = GDF_CreateThread(io_thread, this);
}

ServerNetManager::~ServerNetManager()
{
    io_active = false;

    GDF_JoinThread(recv_thread);
    GDF_DestroyThread(recv_thread);

    GDF_DestroyMutex(incoming_mutex);
    GDF_DestroyMutex(broadcast_mutex);
    GDF_DestroyMutex(clients_mutex);

    enet_host_destroy(host);
}
