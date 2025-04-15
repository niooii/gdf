#include <services/global.h>

#ifndef GDF_CLIENT_BUILD

#include <server/server.h>
namespace Services {
    World* Services::world_ptr() {
        return SERVER.server_world->world_ptr();
    }
}

#else

#include <client/app.h>

namespace Services {
    World* Services::world_ptr() {
        return APP.client_world->world_ptr();
    }
}

#endif