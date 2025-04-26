#include <server/net.h>
#include <gdfe/os/thread.h>
#include <prelude.h>
#include <game/events/defs.h>
#include <game/net.h>

using namespace Services;
static unsigned long io_thread(void* args) {
    ServerNetManager* server = (ServerNetManager*) args;
    GDF_InitThreadLogging("Server:Net");

    const GDF_Stopwatch throttle_timer = GDF_StopwatchCreate();
    // Make loop happen at a fixed rate of 2ms per iteration
    constexpr f64 throttle_secs = 0.002;

    LOG_INFO("Listening on port %d", server->port);
    ENetEvent event;
    do
    {
        GDF_StopwatchReset(throttle_timer);
        for (auto& [uuid, client] : server->clients)
        {
            GDF_LockMutex(client->outgoing_mutex);
            for (const auto& outgoing : client->outgoing_queue)
            {
                std::string serialized {Events::serialize(outgoing)};

                ENetPacket* packet = enet_packet_create(
                    serialized.c_str(),
                    serialized.length() + 1,
                    ENET_PACKET_FLAG_RELIABLE
                );

                enet_peer_send(client->peer, 0, packet);
            }
            client->outgoing_queue.clear();
            GDF_ReleaseMutex(client->outgoing_mutex);
        }

        GDF_LockMutex(server->broadcast_mutex);
        for (auto& outgoing : server->broadcast_queue)
        {
            std::string serialized {Events::serialize(outgoing)};

            ENetPacket* packet = enet_packet_create(
                serialized.c_str(),
                serialized.length() + 1,
                ENET_PACKET_FLAG_RELIABLE
            );

            enet_host_broadcast(server->host, 0, packet);
        }
        server->broadcast_queue.clear();
        GDF_ReleaseMutex(server->broadcast_mutex);

        while (enet_host_service(server->host, &event, 0) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                LOG_DEBUG("A new client connected from %x:%u.\n",
                    event.peer->address.host, event.peer->address.port);
                event.peer->data = GDF_Malloc(sizeof(ConnectedClient), GDF_MEMTAG_UNKNOWN);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                {
                    // LOG_DEBUG("A packet of length %u containing %s was received from %s on channel %u.\n",
                    //     event.packet->dataLength,
                    //     event.packet->data,
                    //     (char*) event.peer->data,
                    //     event.channelID
                    // );

                    ConnectedClient* client = (ConnectedClient*)event.peer->data;
                    try
                    {
                        auto recv_event = Events::deserialize(
                        {
                                (char*)event.packet->data,
                                event.packet->dataLength
                            }
                        );

                        if (recv_event->source != ProgramType::Client)
                        {
                            LOG_WARN("Rejected packet for invalid packet source.");
                            goto DESTROY_PACKET;
                        }

                        if (recv_event->connect_event)
                        {
                            ClientConnectionEvent* conn =
                                    (dynamic_cast<ClientConnectionEvent*>(recv_event.get()));

                            if (LIKELY(conn))
                            {
                                client->auth = conn->auth;
                                // TODO! get uuid and name from some centralized server
                                // or something? idk.
                                client->uuid = conn->uuid;
                                client->name = "NAMELESS... FORMLESS...";

                                LOG_DEBUG("Initialized connection information for %s!", conn->uuid.c_str());
                            }
                            else
                            {
                                LOG_ERR("Recieved malformed connection packet, treating like a regular packet..");
                            }
                        }

                        // TODO! unnecessary allocation but not the main issue prob
                        recv_event->source_uuid = client->uuid;

                        GDF_LockMutex(server->incoming_mutex);
                        server->incoming_queue.push_back(std::move(recv_event));
                        GDF_ReleaseMutex(server->incoming_mutex);
                    } catch (const ser20::Exception& e)
                    {
                        LOG_ERR("Failed to deserialize a packet from %s", client->uuid.c_str());
                    }
                    DESTROY_PACKET:
                    enet_packet_destroy(event.packet);
                }
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                LOG_DEBUG("%s disconnected.\n", (char*) event.peer->data);
                GDF_Free(event.peer->data);
                break;

            case ENET_EVENT_TYPE_NONE:
                break;

            default:
                LOG_ERR("BAD ENET EVENT.");
            }
        }
        if (
            const f64 overflow = GDF_StopwatchSleepUntil(throttle_timer, throttle_secs);
            overflow > 0
        )
            LOG_WARN("Server fell behind by %lf secs :(", overflow);
    } while (server->io_active);

    GDF_StopwatchDestroy(throttle_timer);

    return 0;
}

ServerNetManager::ServerNetManager(u16 port, u16 max_clients) {
    ENetAddress addr = {
        .port = port,
        .host = ENET_HOST_ANY
    };

    clients.reserve(32);

    Events::subscribe<TestMsgEvent>([](auto& event)
    {
        LOG_INFO("Words from %s: \"%s\"", event.source_uuid.c_str(), event.message.c_str());
    });

    io_active = true;
    this->port = port;

    incoming_mutex = GDF_CreateMutex();
    broadcast_mutex = GDF_CreateMutex();

    if (enet_initialize() != 0) {
        LOG_FATAL("An error occurred while initializing ENet");
    }

    this->host = enet_host_create(
        &addr,
        max_clients,
        2,
        0,
        0
    );

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

    enet_host_destroy(host);
}