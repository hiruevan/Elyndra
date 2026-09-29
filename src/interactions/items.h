#ifndef ITEM_H
#define ITEM_H

#include <stdint.h>

// Every distinct item in the game. Add one entry per real item.
typedef enum {
    ITEM_NONE = 0,
    ITEM_HERB,
    ITEM_TONIC,
    ITEM_STONE_TABLET,
    ITEM_RUSTY_SWORD,
    ITEM_LEATHER_ARMOR,
    ITEM_COUNT   // keep last, used for bounds checks
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
    uint8_t       stackable;   // 1 = stacks, 0 = unique
    union {
        struct { uint8_t heal_amount; } consumable;
        struct { uint8_t attack; }      weapon;
        struct { uint8_t defense; }     armor;
    } stats;
} ItemDef;

extern const ItemDef item_defs[ITEM_COUNT];

#endif