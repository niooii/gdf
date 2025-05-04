#include <game/events/defs.h>
#include <server/server.h>

Server SERVER{};

void server_init()
{
    SERVER.net = new ServerNetManager{ GDF_SERVER_PORT, 64 };

    // Services::Events::reject_dispatch_if<ChunkUpdateEvent>(
    //     [](auto event) {
    //     return event.source == ProgramType::Client;
    // });

    auto tmp            = WorldServerCreateInfo{};
    SERVER.server_world = new ServerWorld{ SERVER.net };

    if (SERVER.global_semaphore_name)
    {
        SERVER.global_semaphore = GDF_GetSemaphore(SERVER.global_semaphore_name);
        LOG_FATAL("Could not acquire global semaphore.. this probably was not supposed to happen.");
        LOG_FATAL("Supplied sempahore name: %s", SERVER.global_semaphore_name);
    }
}

void server_destroy() { delete SERVER.net; }

GDF_BOOL server_update(const GDF_AppState* app_state, f64 dt, void* state)
{
    Services::Time::detail::_internal_dt = dt;
    Server* server                       = (Server*)state;

    server->net->update();
    Services::Events::flush();

    auto test = Services::Events::create_event<ChunkLoadEvent>();
    server->net->send_to("TESTUUID ABABABA", std::move(test));

    server->server_world->update(dt);

    return GDF_TRUE;
}
