#include <server/net.h>

Server::Server(u16 port, u16 max_clients) {
    ENetAddress addr = {
            .port = port,
            .host = ENET_HOST_ANY
        };

        if (enet_initialize() != 0) {
            LOG_FATAL("An error occurred while initializing ENet.\n");
        }

        ENetHost* host = enet_host_create(
            &addr,
            max_clients,
            2,
            0,
            0
        );

        if (host == NULL)
        {
            LOG_FATAL("An error occurred while creating the server host.\n");
        }
}