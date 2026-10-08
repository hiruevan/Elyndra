#include "items.h"

// All the item definitions (List of all items is found in items.h)
const ItemDef item_defs[ITEM_COUNT] = {

    [ITEM_NONE] = {
        .name        = "None",
        .description = "",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 0,
        .value       = 0
    },

    /* =========================
       CONSUMABLES
       ========================= */

    [ITEM_HERB] = {
        .name        = "Herb",
        .description = "Restores a small amount of HP.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .value       = 20,
        .stats.consumable = { .heal_amount = 20 }
    },

    [ITEM_TONIC] = {
        .name        = "Tonic",
        .description = "Restores a moderate amount of HP.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .value       = 35,
        .stats.consumable = { .heal_amount = 50 }
    },

    [ITEM_MEGA_TONIC] = {
        .name        = "Mega Tonic",
        .description = "Restores a large amount of HP.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .value       = 100,
        .stats.consumable = { .heal_amount = 150 }
    },

    [ITEM_ETHER] = {
        .name        = "Ether",
        .description = "Restores a small amount of MP.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .value       = 50,
        .stats.consumable = { .mp_regain = 10 }
    },

    [ITEM_HIGH_ETHER] = {
        .name        = "High Ether",
        .description = "Restores a large amount of MP.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .value       = 150,
        .stats.consumable = { .mp_regain = 30 }
    },

    [ITEM_MEGA_ETHER] = {
        .name        = "Mega Ether",
        .description = "A legendary MP restoration artifact.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .value       = 275,
        .stats.consumable = { .mp_regain = 50 }
    },

    [ITEM_ELIXIR] = {
        .name        = "Elixir",
        .description = "Restores a tremendous amount of HP.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .value       = 500,
        .stats.consumable = { .heal_amount = 500 }
    },

    [ITEM_MEGA_ELIXIR] = {
        .name        = "Mega Elixir",
        .description = "A legendary restorative medicine.",
        .category    = CATEGORY_CONSUMABLE,
        .stackable   = 1,
        .value       = 1200,
        .stats.consumable = { .heal_amount = 999 }
    },

    /* ---------------------------------
    Other consumables (not implemented)
    --------------------------------- */

    // [ITEM_ANTIDOTE] = {
    //     .name        = "Antidote",
    //     .description = "Cures poison.",
    //     .category    = CATEGORY_CONSUMABLE,
    //     .stackable   = 1,
    //     .value       = 25
    // },

    // [ITEM_PHOENIX_DOWN] = {
    //     .name        = "Phoenix Down",
    //     .description = "Revives a fallen ally.",
    //     .category    = CATEGORY_CONSUMABLE,
    //     .stackable   = 1,
    //     .value       = 200
    // },

    // [ITEM_SMOKE_BOMB] = {
    //     .name        = "Smoke Bomb",
    //     .description = "Creates an escape from battle.",
    //     .category    = CATEGORY_CONSUMABLE,
    //     .stackable   = 1,
    //     .value       = 75
    // },

    // [ITEM_POWER_SEED] = {
    //     .name        = "Power Seed",
    //     .description = "Permanently increases attack.",
    //     .category    = CATEGORY_CONSUMABLE,
    //     .stackable   = 1,
    //     .value       = 500
    // },

    // [ITEM_GUARD_SEED] = {
    //     .name        = "Guard Seed",
    //     .description = "Permanently increases defense.",
    //     .category    = CATEGORY_CONSUMABLE,
    //     .stackable   = 1,
    //     .value       = 500
    // },

    /* =========================
       KEY ITEMS
       ========================= */

    [ITEM_STONE_TABLET] = {
        .name        = "Stone Tablet",
        .description = "This may be useful later.",
        .category    = CATEGORY_KEY,
        .stackable   = 0,
        .value       = 0
    },

    [ITEM_ANCIENT_KEY] = {
        .name        = "Ancient Key",
        .description = "An old key covered in strange markings.",
        .category    = CATEGORY_KEY,
        .stackable   = 0,
        .value       = 0
    },

    [ITEM_CRYSTAL_SHARD] = {
        .name        = "Crystal Shard",
        .description = "A fragment of a mysterious crystal.",
        .category    = CATEGORY_KEY,
        .stackable   = 0,
        .value       = 0
    },

    /* =========================
       WEAPONS
       ========================= */

    [ITEM_RUSTY_SWORD] = {
        .name        = "Rusty Sword",
        .description = "Don't cut yourself on it!",
        .category    = CATEGORY_WEAPON,
        .stackable   = 0,
        .value       = 80,
        .stats.weapon = { .attack = 3, .weight = 2 }
    },

    [ITEM_IRON_SWORD] = {
        .name        = "Iron Sword",
        .description = "A dependable sword made of iron.",
        .category    = CATEGORY_WEAPON,
        .stackable   = 0,
        .value       = 250,
        .stats.weapon = { .attack = 8, .weight = 3 }
    },

    [ITEM_STEEL_SWORD] = {
        .name        = "Steel Sword",
        .description = "A sturdy sword forged from steel.",
        .category    = CATEGORY_WEAPON,
        .stackable   = 0,
        .value       = 600,
        .stats.weapon = { .attack = 15, .weight = 4 }
    },

    [ITEM_FLAME_BLADE] = {
        .name        = "Flame Blade",
        .description = "A sword said to contain the power of fire.",
        .category    = CATEGORY_WEAPON,
        .stackable   = 0,
        .value       = 1200,
        .stats.weapon = { .attack = 24, .weight = 4 }
    },

    [ITEM_FROST_BLADE] = {
        .name        = "Frost Blade",
        .description = "Its edge is cold enough to freeze water.",
        .category    = CATEGORY_WEAPON,
        .stackable   = 0,
        .value       = 1200,
        .stats.weapon = { .attack = 24, .weight = 4 }
    },

    [ITEM_STORM_SWORD] = {
        .name        = "Storm Sword",
        .description = "Crackling energy surrounds its blade.",
        .category    = CATEGORY_WEAPON,
        .stackable   = 0,
        .value       = 1800,
        .stats.weapon = { .attack = 32, .weight = 3 }
    },

    [ITEM_KINGS_BLADE] = {
        .name        = "King's Blade",
        .description = "A legendary weapon of ancient royalty.",
        .category    = CATEGORY_WEAPON,
        .stackable   = 0,
        .value       = 3500,
        .stats.weapon = { .attack = 45, .weight = 5 }
    },

    [ITEM_VOID_EDGE] = {
        .name        = "Void Edge",
        .description = "A blade that seems to cut through reality.",
        .category    = CATEGORY_WEAPON,
        .stackable   = 0,
        .value       = 6000,
        .stats.weapon = { .attack = 60, .weight = 3 }
    },

    /* =========================
       ARMOR
       ========================= */
    
    [ITEM_CLOTH] = {
        .name        = "Cloak",
        .description = "A simple article of clothing.",
        .category    = CATEGORY_ARMOR,
        .stackable   = 0,
        .value       = 60,
        .stats.armor = { .defense = 1, .weight = 1 }
    },

    [ITEM_LEATHER_ARMOR] = {
        .name        = "Leather Armor",
        .description = "Light protection.",
        .category    = CATEGORY_ARMOR,
        .stackable   = 0,
        .value       = 120,
        .stats.armor = { .defense = 3, .weight = 1 }
    },

    [ITEM_IRON_ARMOR] = {
        .name        = "Iron Armor",
        .description = "Basic armor made of iron.",
        .category    = CATEGORY_ARMOR,
        .stackable   = 0,
        .value       = 300,
        .stats.armor = { .defense = 8, .weight = 3 }
    },

    [ITEM_STEEL_ARMOR] = {
        .name        = "Steel Armor",
        .description = "Heavy armor offering excellent protection.",
        .category    = CATEGORY_ARMOR,
        .stackable   = 0,
        .value       = 700,
        .stats.armor = { .defense = 16, .weight = 5 }
    },

    [ITEM_MYSTIC_ROBE] = {
        .name        = "Mystic Robe",
        .description = "A robe woven with mysterious magic.",
        .category    = CATEGORY_ARMOR,
        .stackable   = 0,
        .value       = 1100,
        .stats.armor = { .defense = 21, .weight = 2 }
    },

    [ITEM_DRAGON_SCALE] = {
        .name        = "Dragon Scale",
        .description = "Armor fashioned from the scales of a dragon.",
        .category    = CATEGORY_ARMOR,
        .stackable   = 0,
        .value       = 2500,
        .stats.armor = { .defense = 32, .weight = 4 }
    },

    [ITEM_GUARDIAN_MAIL] = {
        .name        = "Guardian Mail",
        .description = "Armor once worn by a legendary guardian.",
        .category    = CATEGORY_ARMOR,
        .stackable   = 0,
        .value       = 4000,
        .stats.armor = { .defense = 45, .weight = 5 }
    },

    [ITEM_VOID_ARMOR] = {
        .name        = "Void Armor",
        .description = "Armor forged from an unknown dark material.",
        .category    = CATEGORY_ARMOR,
        .stackable   = 0,
        .value       = 6500,
        .stats.armor = { .defense = 60, .weight = 3 }
    }
};