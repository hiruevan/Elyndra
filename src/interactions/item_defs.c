#include "items.h"

// All the item definitions (List of all items is found in items.h)
const ItemDef item_defs[ITEM_COUNT] = {
    [ITEM_NONE] = {
        .name        = "None",
        .description = "",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 0,
    },
    [ITEM_HERB] = {
        .name        = "Herb",
        .description = "Restores some health.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .stats.consumable = { .heal_amount = 20 }
    },
    [ITEM_TONIC] = {
        .name        = "Tonic",
        .description = "Restores lots of health.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .stats.consumable = { .heal_amount = 50 }
    },
    [ITEM_STONE_TABLET] = {
        .name        = "Stone Tablet",
        .description = "This may be useful later.",
        .category    = CATEGORY_KEY,
        .stackable   = 0,
    },
    [ITEM_RUSTY_SWORD] = {
        .name        = "Rusty Sword",
        .description = "Don't cut yourself on it!",
        .category    = CATEGORY_WEAPON,
        .stackable   = 0,
        .stats.weapon = { .attack = 5}
    },
    [ITEM_LEATHER_ARMOR] = {
        .name        = "Leather Armor",
        .description = "Light protection.",
        .category    = CATEGORY_ARMOR,
        .stackable   = 0,
        .stats.armor = { .defense = 3 }
    },
};