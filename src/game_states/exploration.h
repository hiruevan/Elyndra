#ifndef EXPLORATION_H
#define EXPLORATION_H

#include "exploration.h"
#include "libs/rendering.h"
#include "libs/player.h"
#include "libs/interactable.h"
#include "game_state.h"

#define ENCOUNTER_GRACE_STEPS 3

void exploration_logic(Renderer *renderer, Camera *camera, Player *player, GameState *current_state, TileMap *current_map, Tile *tileset, AreaId *current_area);

#endif