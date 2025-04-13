#pragma once

enum class ProgramType {
    Client = 0,
    Server
};

#ifdef GDF_CLIENT_BUILD
constexpr ProgramType CURR_PROGRAM_TYPE = ProgramType::Client;
#else
constexpr ProgramType CURR_PROGRAM_TYPE = ProgramType::Server;
#endif