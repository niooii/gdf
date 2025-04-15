#include <game/events/defs.h>
#include <server/server.h>

Server SERVER{};

void server_init()
{
    Services::Events::reject_dispatch_if<ChunkUpdateEvent>(
        [](auto event) {
        return event.source == ProgramType::Client;
    });

    auto tmp = WorldServerCreateInfo{};
    SERVER.server_world = new ServerWorld{tmp};

    if (SERVER.global_semaphore_name)
    {
        SERVER.global_semaphore = GDF_GetSemaphore(SERVER.global_semaphore_name);
        LOG_FATAL("Could not acquire global semaphore.. this probably was not supposed to happen.");
        LOG_FATAL("Supplied sempahore name: %s", SERVER.global_semaphore_name);
    }
}

void server_destroy()
{

}

GDF_BOOL server_update(const GDF_AppState* app_state, f64 dt, void* state)
{
    Server* server = (Server*)state;
    server->server_world->tick();

    Services::Events::flush();
    return GDF_TRUE;
}
