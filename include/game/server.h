#pragma once
#include <gdfe/core.h>
#include <gdfe/os/socket.h>
#include <gdfe/os/thread.h>
#include <game/world.h>
#include <gdfe/collections/hashmap.h>
#include <gdfe/collections/list.h>

// NOT MINECRFT CLONE I SWEAR
#define SERVER_PORT 25567

typedef enum WORLDSERVER_EXIT_CODE {
    WORLDSERVER_GRACEFUL,
    WORLDSERVER_NOT_INITIALIZED,
    WORLDSERVER_SOCKET_ERR,
    WORLDSERVER_WORLD_CREATION_ERR,
    WORLDSERVER_WORLD_LOAD_ERR,
    WORLDSERVER_SYNC_ERR
} WORLDSERVER_EXIT_CODE;

typedef enum PACKET_TYPE {
    PACKET_HANDSHAKE,
    PACKET_DISCONNECT,
    PACKET_PLAYER_POSITION,
    PACKET_BLOCK_UPDATE
    // etc etc
} PACKET_TYPE;

typedef struct ClientInfo {
    GDF_Socket* client_socket;
    GDF_BOOL fully_loaded;
} ClientInfo;

typedef struct WorldServer {
    World world;
    GDF_Mutex clients_mutex;
    GDF_LIST(ClientInfo) clients;
    // Map of uid -> ClientInfo for quick access.
    GDF_HashMap client_map;
    GDF_BOOL alive;
    GDF_BOOL initialized;
    u8 max_clients;
    u8 connected_clients;
} WorldServer;

// World path must be a valid path to a world or NULL, if NULL then create_info must not be NULL.
typedef struct WorldServerStartInfo {
    const char* world_path;
    WorldCreateInfo* create_info;
    u8 max_clients;
} WorldServerStartInfo;

// Starts 
GDF_BOOL world_server_init(WorldServerStartInfo* start_info, WorldServer* ctx);

WORLDSERVER_EXIT_CODE world_server_run(WorldServer* ctx);