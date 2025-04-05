#include <client/net.h>
#include <game/events.h>

static unsigned long io_thread(void* args)
{
    ServerConnection* conn = (ServerConnection*) args;
    GDF_InitThreadLogging("Client:Net");

    LOG_INFO("Listening for server events..");

    ENetEvent event;
    auto& event_manager = EventManager::get_instance();
    for(;;)
    {
        if (!conn->io_active)
            return 0;

        GDF_LockMutex(conn->outgoing_mutex);
        for (auto& outgoing : conn->outgoing_queue)
        {
            std::string serialized {event_manager.serialize(outgoing)};

            ENetPacket* packet = enet_packet_create(
                serialized.c_str(),
                serialized.length() + 1,
                ENET_PACKET_FLAG_RELIABLE
            );

            enet_peer_send(conn->peer, 0, packet);
            LOG_WARN("Packets sent!!!");
        }
        conn->outgoing_queue.clear();
        GDF_ReleaseMutex(conn->outgoing_mutex);

        while (enet_host_service(conn->client, &event, 0) > 0)
        {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                {
                    LOG_DEBUG("A packet of length %u containing %s was received from server on channel %u.\n",
                    event.packet->dataLength,
                    event.packet->data,
                    event.channelID);

                    auto recv_event = event_manager.deserialize(
                        {
                            (char*)event.packet->data,
                            event.packet->dataLength
                        }
                    );

                    enet_packet_destroy(event.packet);

                    if (recv_event->source != ProgramType::Server)
                    {
                        LOG_WARN("Rejected packet for invalid packet source.");
                        continue;
                    }

                    GDF_LockMutex(conn->incoming_mutex);
                    conn->incoming_queue.push_back(std::move(recv_event));
                    GDF_ReleaseMutex(conn->incoming_mutex);
                }
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                LOG_DEBUG("Server disconnected.\n");
                return 0;

            default:
                break;
            }
        }
    }
}

ServerConnection::ServerConnection(const char* addr, u16 port)
{
    io_active = true;
    this->port = port;
    this->addr = addr;

    incoming_mutex = GDF_CreateMutex();
    outgoing_mutex = GDF_CreateMutex();

    if (enet_initialize() != 0) {
        LOG_FATAL("An error occurred while initializing ENet");
    }

    constexpr u32 channels = 2;

    client = enet_host_create(NULL, 1, channels, 0, 0);
    if (client == NULL) {
        LOG_FATAL("An error occurred while creating the client")
    }

    ENetAddress address{};
    enet_address_set_host(&address, addr);
    address.port = port;

    this->peer = enet_host_connect(client, &address, channels, 0);

    if (this->peer == NULL) {
        LOG_ERR("No available peers for initiating an ENet connection");
        enet_host_destroy(client);
        enet_deinitialize();
        throw std::runtime_error("Connection to server failed");
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT) {
        LOG_INFO("Connection to server succeeded");
    }
    else {
        enet_peer_reset(peer);
        enet_host_destroy(client);
        enet_deinitialize();
        throw std::runtime_error("Connection to server failed");
    }

    recv_thread = GDF_CreateThread(io_thread, this);
}

ServerConnection::~ServerConnection()
{
    io_active = false;

    GDF_JoinThread(recv_thread);
    GDF_DestroyThread(recv_thread);

    GDF_DestroyMutex(incoming_mutex);
    GDF_DestroyMutex(outgoing_mutex);

    enet_peer_reset(peer);
    enet_peer_disconnect_now(peer, 0);

    enet_host_destroy(client);
    enet_deinitialize();
}

void ServerConnection::send(std::unique_ptr<EventBase> unique_ptr)
{
    unique_ptr->source = ProgramType::Client;
    GDF_LockMutex(outgoing_mutex);
    outgoing_queue.push_back(std::move(unique_ptr));
    GDF_ReleaseMutex(outgoing_mutex);
}

void ServerConnection::dispatch_incoming()
{
    EventManager& events = EventManager::get_instance();

    GDF_LockMutex(incoming_mutex);
    for (auto& event : incoming_queue)
    {
        event->dispatch_self(events);
    }

    incoming_queue.clear();
    GDF_ReleaseMutex(incoming_mutex);
}