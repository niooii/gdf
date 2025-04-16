#pragma once

#include <gdfe/core.h>
#include <game/items/item.h>
#include "item_slot.h"

namespace Components {
    struct Inventory {

    };
}

typedef struct GDF_Inventory {
    GDF_ItemSlot* item_slots[36];
} GDF_Inventory; 

GDF_Inventory* GDF_MakeInventory();
