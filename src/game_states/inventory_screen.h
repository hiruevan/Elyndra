#ifndef INVENTORY_SCREEN_H
#define INVENTORY_SCREEN_H
 
#include <stdbool.h>
 
#include "game_state.h"
#include "../libs/player.h"
#include "../libs/rendering.h"

bool status_apply_item(PartyMember *m, const ItemDef *def);
void inventory_screen_set_tab(uint8_t tab);
bool inventory_screen(Player *player,
    GameState *current_state, TileMap *current_map, Tile *tileset, Renderer *renderer, Camera *camera);
 
#endif
 