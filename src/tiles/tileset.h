#ifndef TILESET_H
#define TILESET_H

#pragma once
#include "../libs/tiles.h"
#include "tile_sprites.h"

Tile tileset[MAX_TILES];

void loadTileset() {
    tiles_init(tileset);

    tiles_set(tileset, 0, bricks_sprite);
    tiles_set(tileset, 1, bricks_with_overhang_sprite);
    tiles_set(tileset, 2, cave_sprite);
    tiles_set(tileset, 3, chest_closed_sprite);
    tiles_set(tileset, 4, chest_open_sprite);
    tiles_set(tileset, 5, chimney_left_sprite);
    tiles_set(tileset, 6, chimney_right_sprite);
    tiles_set(tileset, 7, cobblestone_sprite);
    tiles_set(tileset, 8, dirt_sprite);
    tiles_set(tileset, 9, door_sprite);
    tiles_set(tileset, 10, grass_sprite);
    tiles_set(tileset, 11, house_sprite);
    tiles_set(tileset, 12, hut_sprite);
    tiles_set(tileset, 13, mountain0_sprite);
    tiles_set(tileset, 14, mountain1_sprite);
    tiles_set(tileset, 15, mountain2_sprite);
    tiles_set(tileset, 16, mountain3_sprite);
    tiles_set(tileset, 17, mountain4_sprite);
    tiles_set(tileset, 18, mountain5_sprite);
    tiles_set(tileset, 19, mountain6_sprite);
    tiles_set(tileset, 20, mountain7_sprite);
    tiles_set(tileset, 21, mountain8_sprite);
    tiles_set(tileset, 22, roof_left0_sprite);
    tiles_set(tileset, 23, roof_left1_sprite);
    tiles_set(tileset, 24, roof_left2_sprite);
    tiles_set(tileset, 25, roof_peak0_sprite);
    tiles_set(tileset, 26, roof_peak1_sprite);
    tiles_set(tileset, 27, roof_peak2_sprite);
    tiles_set(tileset, 28, roof_right0_sprite);
    tiles_set(tileset, 29, roof_right1_sprite);
    tiles_set(tileset, 30, roof_right2_sprite);
    tiles_set(tileset, 31, sheetrock_sprite);
    tiles_set(tileset, 32, tower0_sprite);
    tiles_set(tileset, 33, tower1_sprite);
    tiles_set(tileset, 34, tower2_sprite);
    tiles_set(tileset, 35, tower3_sprite);
    tiles_set(tileset, 36, tower4_sprite);
    tiles_set(tileset, 37, tree_sprite);
    tiles_set(tileset, 38, window_left_sprite);
    tiles_set(tileset, 39, window_right_sprite);
}

#endif

/* 
const TILE_NAMES = ['bricks','bricks_with_overhang','cave','chest_closed','chest_open','chimney_left','chimney_right','cobblestone','dirt','door','grass','house','hut','mountain0','mountain1','mountain2','mountain3','mountain4','mountain5','mountain6','mountain7','mountain8','roof_left0','roof_left1','roof_left2','roof_peak0','roof_peak1','roof_peak2','roof_right0','roof_right1','roof_right2','sheetrock','tower0','tower1','tower2','tower3','tower4','tree','window_left','window_right'];
*/
