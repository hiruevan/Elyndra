#ifndef PAUSE_SCREEN_H
#define PAUSE_SCREEN_H
 
#include <stdbool.h>
 
#include "libs/player.h"
#include "libs/rendering.h"
#include "game_state.h"

bool pause_screen(Renderer *renderer, Camera *camera, Player *player, TileMap *current_map, GameState *current_state, Tile *tileset);
 
#endif
 