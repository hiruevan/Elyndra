#ifndef TILESET_H
#define TILESET_H

#pragma once
#include "../libs/tiles.h"
#include <resources.h>

Tile tileset[MAX_TILES];

void loadTileset() {
    tiles_init(tileset);

    tiles_set(tileset, 0, sprites_tiles_bricks_sprite);
    tiles_set(tileset, 1, sprites_tiles_bricks_with_overhang_sprite);
    tiles_set(tileset, 2, sprites_tiles_cave_sprite);
    tiles_set(tileset, 3, sprites_tiles_chest_closed_sprite);
    tiles_set(tileset, 4, sprites_tiles_chest_open_sprite);
    tiles_set(tileset, 5, sprites_tiles_chimney_left_sprite);
    tiles_set(tileset, 6, sprites_tiles_chimney_right_sprite);
    tiles_set(tileset, 7, sprites_tiles_cobblestone_sprite);
    tiles_set(tileset, 8, sprites_tiles_dirt_sprite);
    tiles_set(tileset, 9, sprites_tiles_door_sprite);
    tiles_set(tileset, 10, sprites_tiles_grass_sprite);
    tiles_set(tileset, 11, sprites_tiles_house_sprite);
    tiles_set(tileset, 12, sprites_tiles_hut_sprite);
    tiles_set(tileset, 13, sprites_tiles_mountain0_sprite);
    tiles_set(tileset, 14, sprites_tiles_mountain1_sprite);
    tiles_set(tileset, 15, sprites_tiles_mountain2_sprite);
    tiles_set(tileset, 16, sprites_tiles_mountain3_sprite);
    tiles_set(tileset, 17, sprites_tiles_mountain4_sprite);
    tiles_set(tileset, 18, sprites_tiles_mountain5_sprite);
    tiles_set(tileset, 19, sprites_tiles_mountain6_sprite);
    tiles_set(tileset, 20, sprites_tiles_mountain7_sprite);
    tiles_set(tileset, 21, sprites_tiles_mountain8_sprite);
    tiles_set(tileset, 22, sprites_tiles_roof_left0_sprite);
    tiles_set(tileset, 23, sprites_tiles_roof_left1_sprite);
    tiles_set(tileset, 24, sprites_tiles_roof_left2_sprite);
    tiles_set(tileset, 25, sprites_tiles_roof_peak0_sprite);
    tiles_set(tileset, 26, sprites_tiles_roof_peak1_sprite);
    tiles_set(tileset, 27, sprites_tiles_roof_peak2_sprite);
    tiles_set(tileset, 28, sprites_tiles_roof_right0_sprite);
    tiles_set(tileset, 29, sprites_tiles_roof_right1_sprite);
    tiles_set(tileset, 30, sprites_tiles_roof_right2_sprite);
    tiles_set(tileset, 31, sprites_tiles_sheetrock_sprite);
    tiles_set(tileset, 32, sprites_tiles_tower0_sprite);
    tiles_set(tileset, 33, sprites_tiles_tower1_sprite);
    tiles_set(tileset, 34, sprites_tiles_tower2_sprite);
    tiles_set(tileset, 35, sprites_tiles_tower3_sprite);
    tiles_set(tileset, 36, sprites_tiles_tower4_sprite);
    tiles_set(tileset, 37, sprites_tiles_tree_sprite);
    tiles_set(tileset, 38, sprites_tiles_window_left_sprite);
    tiles_set(tileset, 39, sprites_tiles_window_right_sprite);
}

#endif

/* 
const TILE_NAMES = ['bricks','bricks_with_overhang','cave','chest_closed','chest_open','chimney_left','chimney_right','cobblestone','dirt','door','grass','house','hut','mountain0','mountain1','mountain2','mountain3','mountain4','mountain5','mountain6','mountain7','mountain8','roof_left0','roof_left1','roof_left2','roof_peak0','roof_peak1','roof_peak2','roof_right0','roof_right1','roof_right2','sheetrock','tower0','tower1','tower2','tower3','tower4','tree','window_left','window_right'];
*/
