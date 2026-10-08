#ifndef INVENTORY_SCREEN_H
#define INVENTORY_SCREEN_H
 
#include <stdbool.h>
 
#include "game_state.h"
#include "../libs/player.h"
#include "../libs/rendering.h"

bool status_apply_item(PartyMember *m, const ItemDef *def);
void inventory_screen_set_tab(uint8_t tab);
bool inventory_screen(Player *player, uint8_t
    *inv_prev_up, uint8_t *inv_prev_down, uint8_t *inv_prev_left, uint8_t *inv_prev_right, uint8_t *inv_prev_action, uint8_t *inv_prev_back, uint8_t *inv_prev_inv,
    GameState *current_state, TileMap *current_map, Tile *tileset, Renderer *renderer, Camera *camera);
 
#endif
 