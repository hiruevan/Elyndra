#ifndef TITLE_SCREEN_H
#define TITLE_SCREEN_H

#include "libs/rendering.h"
#include "libs/player.h"
#include "game_state.h"

void title_logic(Renderer *renderer, Camera *camera, Player *player, uint8_t action_key_down, GameState *current_state, TileMap *current_map, Tile *tileset);

#endif
 