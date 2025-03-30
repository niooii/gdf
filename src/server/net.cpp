#include <server/net.h>
#include <gdfe/os/thread.h>
#include <game/prelude.h>

static unsigned long recv_thread(void* args) {
    ServerNetworkManager* server = (ServerNetworkManager*) args;
    GDF_InitThreadLogging("Net");

    LOG_INFO("Listening on port %d", server->port);
    ENetEvent event;
    auto& event_manager = EventManager::get_instance();
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
                {
                    printf("A packet of length %u containing %s was received from %s on channel %u.\n",
                        event.packet->dataLength,
                        event.packet->data,
                        (char*) event.peer->data,
                        event.channelID
                    );

                    auto recv_event = event_manager.deserialize(
                    {
                        (char*)event.packet->data,
                        event.packet->dataLength
                    }
                    );

                    server->incoming_queue.push_back(std::move(recv_event));

                    enet_packet_destroy(event.packet);
                }
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

    GDF_CreateThread(recv_thread, this);
}

ServerNetworkManager::~ServerNetworkManager()
{
    GDF_LockMutex(cont_listening_lock);
    continue_listening = false;
    GDF_ReleaseMutex(cont_listening_lock);

    enet_host_destroy(host);
}
