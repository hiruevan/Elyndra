#ifndef ITEMS_H
#define ITEMS_H

#include <stdint.h>

// Every distinct item in the game. Add one entry per real item.
typedef enum {
    ITEM_NONE,

    // Consumables
    ITEM_HERB,
    ITEM_TONIC,
    ITEM_MEGA_TONIC,
    ITEM_ETHER,
    ITEM_HIGH_ETHER,
    ITEM_MEGA_ETHER,
    ITEM_ELIXIR,
    ITEM_MEGA_ELIXIR,

    // Specail consumables (not implemented)
    // ITEM_ANTIDOTE,
    // ITEM_PHOENIX_DOWN,
    // ITEM_SMOKE_BOMB,
    // ITEM_POWER_SEED,
    // ITEM_GUARD_SEED,

    // Key items
    ITEM_STONE_TABLET,
    ITEM_ANCIENT_KEY,
    ITEM_CRYSTAL_SHARD,

    // Weapons
    ITEM_RUSTY_SWORD,
    ITEM_IRON_SWORD,
    ITEM_STEEL_SWORD,
    ITEM_FLAME_BLADE,
    ITEM_FROST_BLADE,
    ITEM_STORM_SWORD,
    ITEM_KINGS_BLADE,
    ITEM_VOID_EDGE,

    // Armor
    ITEM_CLOTH,
    ITEM_LEATHER_ARMOR,
    ITEM_IRON_ARMOR,
    ITEM_STEEL_ARMOR,
    ITEM_MYSTIC_ROBE,
    ITEM_DRAGON_SCALE,
    ITEM_GUARDIAN_MAIL,
    ITEM_VOID_ARMOR,

    ITEM_COUNT
} ItemId;

// Item type
typedef enum {
    CATEGORY_CONSUMABLE,
    CATEGORY_KEY,
    CATEGORY_WEAPON,
    CATEGORY_ARMOR,
    CATEGORY_COUNT
} ItemCategory;

// Item def struct
typedef struct {
    const char   *name;
    const char   *description;
    ItemCategory  category;
    uint16_t      value; // Sell value
    uint8_t       stackable;   // 1 = stacks, 0 = unique
    union {
        struct { uint16_t heal_amount; uint8_t mp_regain; } consumable;
        struct { uint8_t attack; uint8_t weight; }      weapon;
        struct { uint8_t defense; uint8_t weight; }     armor;
    } stats;
} ItemDef;

extern const ItemDef item_defs[ITEM_COUNT];

#endif