#include <game/events.h>

typedef enum GAME_EVENT {
    CHUNK_LOAD,
    CHUNK_UPDATE,
    CHUNK_UNLOAD,
    TICK
} GAME_EVENT;

void __chunk_loads()
{

}

void __chunk_unloads()
{

}

void __chunk_updates()
{

}

void __ticks()
{

}

void __process_frame_events()
{
    __chunk_loads();
    __chunk_unloads();
    __chunk_updates();
}

void __process_tick_events() 
{
    __ticks();
}

void __add_event(GAME_EVENT event, void* fptr) 
{

}

// public
// void on_chunk_load(CHUNK_LOAD_HANDLER)
// {
//     __add_event(CHUNK_LOAD, handle_chunk_load);
// }
//
// void on_chunk_update(CHUNK_UPDATE_HANDLER)
// {
//     __add_event(CHUNK_UPDATE, handle_chunk_update);
// }
//
// void on_chunk_unload(CHUNK_UNLOAD_HANDLER)
// {
//     __add_event(CHUNK_UNLOAD, handle_chunk_unload);
// }
//
// void on_tick(TICK_HANDLER)
// {
//     __add_event(TICK, handle_tick);
// }