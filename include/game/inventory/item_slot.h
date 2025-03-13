#pragma once

#include <gdfe/../../../gdfe/include/gdfe/core.h>
#include <game/items/item.h>

typedef struct GDF_ItemSlot {
    GDF_Item* item;
    GDF_BOOL has_item;
} GDF_ItemSlot;