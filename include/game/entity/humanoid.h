#pragma once

#include <core.h>
#include <game/movement.h>

#include "entity.h"

typedef struct HumanoidEntity {
    Entity base;
    MovementState movement_state;
    
} HumanoidEntity;

void humanoid_entity_update(HumanoidEntity* humanoid_entity);