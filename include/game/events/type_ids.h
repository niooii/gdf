#pragma once
#include <gdfe/core.h>

enum class EventTypeId : u32 {

    #define DEFINE_EVENT(name, fields, ...) name,
    #include "defs.h"
    #undef DEFINE_EVENT
    MaxEvent
};