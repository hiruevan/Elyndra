#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "../libs/player.h"
#include "../libs/area.h"
#include "../libs/rendering.h"
#include "../game_states/game_state.h"
#include "items.h"

typedef struct {
    ItemId   item;
    uint16_t price;
} ShopEntry;

typedef struct {
    const char      *name;
    const ShopEntry *entries;
    uint8_t          count;
} ShopDef;

extern const ShopDef shop_defs[];

void shop_open(uint8_t shop_id);
bool shop_is_active(void);

/* Returns true while the shop is open, false on the frame it closes. */
bool shop_screen(Player *player, GameState *current_state, TileMap *current_map,
                 Tile *tileset, Renderer *renderer, Camera *camera);