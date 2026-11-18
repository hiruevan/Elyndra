#ifndef PAUSE_SCREEN_H
#define PAUSE_SCREEN_H
 
#include <stdbool.h>
 
#include "../libs/player.h"
#include "../libs/rendering.h"
#include "../libs/save.h"
#include "game_state.h"

bool pause_screen(Renderer *renderer, Camera *camera, Player *player, TileMap *current_map, AreaId current_area, GameState *current_state, Tile *tileset,
    uint8_t save_slot, GameSave *save);
 
#endif
 