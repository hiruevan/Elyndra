#ifndef TILES_H
#define TILES_H

#include <stdint.h>
#include <graphx.h>

#define TILE_SIZE 16

/* Screen dimensions in tiles */
#define MAP_VIEW_WIDTH  20
#define MAP_VIEW_HEIGHT 15

/* Maximum dimensions of a map */
#define MAX_MAP_WIDTH  128
#define MAX_MAP_HEIGHT 128

/* Maximum number of different tiles in the tileset */
#define MAX_TILES 256


/*
 * A tile is simply a sprite plus some metadata.
 *
 * For now, the only metadata we need is the sprite.
 * Collision/properties can be added later without changing
 * how the map itself works.
 */
typedef struct
{
    gfx_sprite_t *sprite;
} Tile;


/*
 * A tile map.
 *
 * Each element of tiles[] is a tile ID referring to the
 * tileset rather than storing an entire sprite.
 */
typedef struct
{
    uint16_t width;
    uint16_t height;

    uint8_t tiles[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];
} TileMap;

/*
 * Initialize a tileset.
 */
void tiles_init(Tile *tileset);

void tilemap_fullset(
    TileMap *map,
    const uint8_t *tiles,
    uint16_t width,
    uint16_t height
);

/*
 * Assign a sprite to a tile ID.
 */
void tiles_set(Tile *tileset, uint8_t id, gfx_sprite_t *sprite);


/*
 * Set one tile in a map.
 */
void tilemap_set(TileMap *map, uint16_t x, uint16_t y, uint8_t tile_id);


/*
 * Get one tile from a map.
 */
uint8_t tilemap_get(const TileMap *map, uint16_t x, uint16_t y);


/*
 * Fill an entire map with one tile.
 */
void tilemap_fill(TileMap *map, uint8_t tile_id);


/*
 * Initialize a map with the specified dimensions.
 */
void tilemap_init(
    TileMap *map,
    uint16_t width,
    uint16_t height,
    uint8_t default_tile
);


/*
 * Draw the entire tile map.
 *
 * screen_x/screen_y specify where the map begins on screen.
 */
void tilemap_draw(
    const TileMap *map,
    const Tile *tileset,
    int16_t screen_x,
    int16_t screen_y
);


/*
 * Draw only a rectangular section of the map.
 *
 * This is used with the camera
 */
void tilemap_draw_region(
    const TileMap *map,
    const Tile *tileset,
    uint16_t start_x,
    uint16_t start_y,
    uint16_t width,
    uint16_t height,
    int16_t screen_x,
    int16_t screen_y
);


/*
 * Draw smaller regions
 *
 * Draws rows and columns for camera movement optimization
*/
void tilemap_draw_column(
    const TileMap *map,
    const Tile *tileset,
    uint16_t world_x,
    uint16_t world_y,
    uint16_t height,
    int16_t screen_x,
    int16_t screen_y
);


/*
 * Draw smaller regions
 *
 * Draws rows and columns for camera movement optimization
*/
void tilemap_draw_row(
    const TileMap *map,
    const Tile *tileset,
    uint16_t world_x,
    uint16_t world_y,
    uint16_t width,
    int16_t screen_x,
    int16_t screen_y
);

#endif