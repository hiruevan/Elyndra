#pragma once
#include "../libs/area.h"
#include "../libs/interactable.h"
#include "items.h"

static Interactable interactables[] =
{
    { AREA_TEST,  33, 34, INTERACT_GOLD_STORE, 500, 0 },
    { AREA_TEST,  35, 36, INTERACT_CHEST, ITEM_LEATHER_ARMOR, 0 },

    { AREA_HOUSE, 2, 6, INTERACT_CHEST, ITEM_STONE_TABLET, 0},
    { AREA_HOUSE, 4, 10, INTERACT_NPC, 0, 0 },
    { AREA_HOUSE, 4, 3, INTERACT_CHEST, ITEM_RUSTY_SWORD, 0 },
    { AREA_HOUSE, 14, 11, INTERACT_CHEST, ITEM_TONIC, 0 },
    { AREA_HOUSE, 8, 3, INTERACT_CHEST, ITEM_HERB, 0 },
};

#define INTERACTABLE_COUNT (sizeof(interactables) / sizeof(interactables[0]))