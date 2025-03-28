#include <server/net.h>
#include <gdfe/os/thread.h>

unsigned long handle_incoming(void* args) {
    ServerNetworkManager* server = (ServerNetworkManager*) args;
    GDF_InitThreadLogging("Net");

    LOG_INFO("Listening for incoming connections on port %d", server->port);
    ENetEvent event;
    for (;;)
    {
        GDF_LockMutex(server->cont_listening_lock);
        if (!server->continue_listening)
            return 0;

        while (enet_host_service(server->host, &event, 1000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %x:%u.\n",
                    event.peer->address.host, event.peer->address.port);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %u containing %s was received from %s on channel %u.\n",
                    event.packet->dataLength,
                    event.packet->data,
                    (char*) event.peer->data,
                    event.channelID);

                enet_peer_send(event.peer, 0, event.packet);

                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("%s disconnected.\n", (char*) event.peer->data);
                break;

            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }

        GDF_ReleaseMutex(server->cont_listening_lock);
    }
}

ServerNetworkManager::ServerNetworkManager(u16 port, u16 max_clients) {
    ENetAddress addr = {
        .port = port,
        .host = ENET_HOST_ANY
    };

    cont_listening_lock = GDF_CreateMutex();
    continue_listening = true;
    this->port = port;

    if (enet_initialize() != 0) {
        LOG_FATAL("An error occurred while initializing ENet.\n");
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
        LOG_FATAL("An error occurred while creating the server host.\n");
    }

    GDF_CreateThread(handle_incoming, this);
}

ServerNetworkManager::~ServerNetworkManager()
{
    GDF_LockMutex(cont_listening_lock);
    continue_listening = false;
    GDF_ReleaseMutex(cont_listening_lock);
}
