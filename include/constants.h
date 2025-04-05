#pragma once

#include <game/events.h>

#ifdef GDF_SERVER_BUILD
constexpr ProgramType CURR_PROGRAM_TYPE = ProgramType::Server;
#else
constexpr ProgramType CURR_PROGRAM_TYPE = ProgramType::Client;
#endif