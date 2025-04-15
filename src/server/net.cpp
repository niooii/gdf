#include <server/net.h>
#include <gdfe/os/thread.h>
#include <prelude.h>
#include <game/events/defs.h>

using namespace Services;
// TODO! throttled thread execution function in gdfe.
// make it now.
// or some horrible horrible macro
// this should execute at a max fixed rate lol
static unsigned long io_thread(void* args) {
    NetworkManager* server = (NetworkManager*) args;
    GDF_InitThreadLogging("Server:Net");

    LOG_INFO("Listening on port %d", server->port);
    ENetEvent event;
    for (;;)
    {
        if (!server->io_active)
            return 0;

        GDF_LockMutex(server->outgoing_mutex);
        for (auto& outgoing : server->outgoing_queue)
        {
            std::string serialized {Events::serialize(outgoing)};

            ENetPacket* packet = enet_packet_create(
                serialized.c_str(),
                serialized.length() + 1,
                ENET_PACKET_FLAG_RELIABLE
            );

            // enet_peer_send(server->peer, 0, packet);
            LOG_WARN("Packets sent!!! (not really actually send packets now)");
        }
        server->outgoing_queue.clear();
        GDF_ReleaseMutex(server->outgoing_mutex);

        while (enet_host_service(server->host, &event, 0) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                LOG_DEBUG("A new client connected from %x:%u.\n",
                    event.peer->address.host, event.peer->address.port);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                {
                    // LOG_DEBUG("A packet of length %u containing %s was received from %s on channel %u.\n",
                    //     event.packet->dataLength,
                    //     event.packet->data,
                    //     (char*) event.peer->data,
                    //     event.channelID
                    // );

                    auto recv_event = Events::deserialize(
                    {
                        (char*)event.packet->data,
                        event.packet->dataLength
                    }
                    );

                    enet_packet_destroy(event.packet);

                    if (recv_event->source != ProgramType::Client)
                    {
                        LOG_WARN("Rejected packet for invalid packet source.");
                        continue;
                    }

                    GDF_LockMutex(server->incoming_mutex);
                    server->incoming_queue.push_back(std::move(recv_event));
                    GDF_ReleaseMutex(server->incoming_mutex);
                }
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                LOG_DEBUG("%s disconnected.\n", (char*) event.peer->data);
                break;

            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }
    }
}

NetworkManager::NetworkManager(u16 port, u16 max_clients) {
    ENetAddress addr = {
        .port = port,
        .host = ENET_HOST_ANY
    };

    Events::subscribe<TestTextEvent>([](auto& event)
    {
        LOG_INFO("Words from client: \"%s\"", event.message.c_str());
    });

    io_active = true;
    this->port = port;

    incoming_mutex = GDF_CreateMutex();
    outgoing_mutex = GDF_CreateMutex();

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

NetworkManager::~NetworkManager()
{
    io_active = false;

    GDF_JoinThread(recv_thread);
    GDF_DestroyThread(recv_thread);

    GDF_DestroyMutex(incoming_mutex);
    GDF_DestroyMutex(outgoing_mutex);

    enet_host_destroy(host);
}

void NetworkManager::dispatch_incoming()
{
    GDF_LockMutex(incoming_mutex);
    for (auto& event : incoming_queue)
    {
        event->queue_dispatch();
    }

    incoming_queue.clear();
    GDF_ReleaseMutex(incoming_mutex);
}