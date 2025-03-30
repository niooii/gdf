#include <client/net.h>
#include <game/events.h>

static unsigned long recv_thread(void* args)
{
    ServerConnection* connection = (ServerConnection*) args;
    GDF_InitThreadLogging("Net");

    LOG_INFO("Listening for server events..");

    ENetEvent event;
    for(;;)
    {
        LOG_INFO("Polling events...");
        while (enet_host_service(connection->client, &event, 1000) > 0)
        {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %u containing %s was received from server on channel %u.\n",
                    event.packet->dataLength,
                    event.packet->data,
                    event.channelID);

                // Clean up the packet
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
    if (enet_initialize() != 0) {
        LOG_FATAL("An error occurred while initializing ENet");
    }

    constexpr u32 channels = 2;

    this->client = enet_host_create(NULL, 1, channels, 0, 0);
    if (this->client == NULL) {
        LOG_FATAL("An error occurred while creating the client")
    }

    ENetAddress address;
    if (enet_address_set_host(&address, addr) != 0)
    {
        LOG_FATAL("GG");
    }
    address.host = ENET_HOST_ANY;
    address.port = port;

    this->peer = enet_host_connect(client, &address, channels, 0);

    if (this->peer == NULL) {
        fprintf(stderr, "No available peers for initiating an ENet connection");
        enet_host_destroy(this->client);
        enet_deinitialize();
        throw std::runtime_error("Connection to server failed");
    }

    // ENetEvent event;
    // if (enet_host_service(client, &event, 5000) > 0 &&
    //     event.type == ENET_EVENT_TYPE_CONNECT) {
    //     LOG_INFO("Connection to server succeeded");
    // }
    // else {
    //     enet_peer_reset(peer);
    //     enet_host_destroy(client);
    //     enet_deinitialize();
    //     throw std::runtime_error("Connection to server failed");
    // }

    GDF_CreateThread(recv_thread, this);
}

ServerConnection::~ServerConnection()
{

}

void ServerConnection::send(std::unique_ptr<EventBase> unique_ptr)
{
    /* Create a reliable packet of size 7 containing "packet\0" */
    ENetPacket * packet = enet_packet_create ("packet",
                                              strlen ("packet") + 1,
                                              ENET_PACKET_FLAG_RELIABLE);

    /* Send the packet to the peer over channel id 0. */
    /* One could also broadcast the packet by         */
    /* enet_host_broadcast (host, 0, packet);         */
    enet_peer_send (peer, 0, packet);
    /* One could just use enet_host_service() instead. */
    enet_host_flush (client);
    LOG_WARN("Packets sent!!!");
}

void ServerConnection::dispatch_incoming()
{

}