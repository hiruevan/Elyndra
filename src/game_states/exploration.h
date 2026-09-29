#ifndef EXPLORATION_H
#define EXPLORATION_H

#include "exploration.h"
#include "libs/rendering.h"
#include "libs/player.h"
#include "libs/interactable.h"
#include "game_state.h"

void exploration_logic(Renderer *renderer, Camera *camera, Player *player, uint8_t action_key_down, uint8_t *prev_action_key, GameState *current_state, TileMap *current_map, Tile *tileset, AreaId *current_area);

#endif