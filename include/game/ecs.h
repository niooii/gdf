#pragma once
#include <vector>
#undef max
#include <entt/entt.hpp>

/* These are aliases for ENTT's types, in case I implement my own somewhere down the line */
namespace ecs {
    using Entity          = entt::entity;
    using Registry        = entt::registry;
    using EntityRegistry  = entt::registry;
    using ComponentTypeId = entt::id_type;
} // namespace ecs
