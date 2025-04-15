#pragma once

/* Allowing for global access to common stuff that server and client both have */
class World;
namespace Services {
    World* world_ptr();
}