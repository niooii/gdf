#include <client/net.h>
#include <game/events.h>

static unsigned long recv_thread_fn(void* args)
{
    ServerConnection* conn = (ServerConnection*) args;
    GDF_InitThreadLogging("Net");

    LOG_INFO("Listening for server events..");

    ENetEvent event;
    auto& event_manager = EventManager::get_instance();
    for(;;)
    {
        if (!conn->continue_listening)
            return 0;
        while (enet_host_service(conn->client, &event, 0) > 0)
        {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %u containing %s was received from server on channel %u.\n",
                    event.packet->dataLength,
                    event.packet->data,
                    event.channelID);

                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("Server disconnected.\n");
                return 0;

            default:
                break;
            }
        }
    }
}

ServerConnection::ServerConnection(const char* addr, u16 port)
{
    continue_listening = true;
    this->port = port;
    this->addr = addr;

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
        fprintf(stderr, "No available peers for initiating an ENet connection");
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

    recv_thread = GDF_CreateThread(recv_thread_fn, this);
}

ServerConnection::~ServerConnection()
{
    continue_listening = false;

    GDF_JoinThread(recv_thread);
    GDF_DestroyThread(recv_thread);

    enet_peer_reset(peer);
    enet_peer_disconnect_now(peer, 0);

    enet_host_destroy(client);
    enet_deinitialize();
}

void ServerConnection::send(std::unique_ptr<EventBase> unique_ptr)
{
    std::string data = EventManager::get_instance().serialize(unique_ptr);
    ENetPacket* packet = enet_packet_create(
        data.c_str(),
        data.length() + 1,
        ENET_PACKET_FLAG_RELIABLE
    );

    enet_peer_send (peer, 0, packet);
    LOG_WARN("Packets sent!!!");
}

void ServerConnection::dispatch_incoming()
{

}