#pragma once

#include <gdfe/../../../gdfe/include/gdfe/core.h>
#include <game/items/item.h>
#include "item_slot.h"

typedef struct GDF_Inventory {
    GDF_ItemSlot* item_slots[36];
} GDF_Inventory; 

GDF_Inventory* GDF_MakeInventory();
