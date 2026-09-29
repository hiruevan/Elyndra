#include "tiles.h"


void tiles_init(Tile *tileset)
{
    uint16_t i;

    for (i = 0; i < MAX_TILES; i++)
    {
        tileset[i].sprite = NULL;
    }
}


void tiles_set(Tile *tileset, uint8_t id, gfx_sprite_t *sprite)
{
    tileset[id].sprite = sprite;
}


void tilemap_init(
    TileMap *map,
    uint16_t width,
    uint16_t height,
    uint8_t default_tile
)
{
    uint32_t i;
    uint32_t size;

    /*
     * Prevent the map from exceeding the allocated array.
     */
    if (width > MAX_MAP_WIDTH)
        width = MAX_MAP_WIDTH;

    if (height > MAX_MAP_HEIGHT)
        height = MAX_MAP_HEIGHT;

    map->width = width;
    map->height = height;

    size = (uint32_t)width * height;

    for (i = 0; i < size; i++)
    {
        map->tiles[i] = default_tile;
    }
}


void tilemap_fullset(
    TileMap *map,
    const uint8_t *tiles,
    uint16_t width,
    uint16_t height
)
{
    uint16_t x;
    uint16_t y;

    if (width > MAX_MAP_WIDTH)
        return;

    if (height > MAX_MAP_HEIGHT)
        return;

    map->width = width;
    map->height = height;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            map->tiles[y * width + x] =
                tiles[y * width + x];
        }
    }
}


void tilemap_fill(TileMap *map, uint8_t tile_id)
{
    uint32_t i;
    uint32_t size;

    size = (uint32_t)map->width * map->height;

    for (i = 0; i < size; i++)
    {
        map->tiles[i] = tile_id;
    }
}


void tilemap_set(
    TileMap *map,
    uint16_t x,
    uint16_t y,
    uint8_t tile_id
)
{
    /*
     * Ignore coordinates outside the map.
     */
    if (x >= map->width || y >= map->height)
        return;

    map->tiles[(uint32_t)y * map->width + x] = tile_id;
}


uint8_t tilemap_get(
    const TileMap *map,
    uint16_t x,
    uint16_t y
)
{
    if (x >= map->width || y >= map->height)
        return 0;

    return map->tiles[(uint32_t)y * map->width + x];
}


void tilemap_draw(
    const TileMap *map,
    const Tile *tileset,
    int16_t screen_x,
    int16_t screen_y
)
{
    uint16_t x;
    uint16_t y;

    for (y = 0; y < map->height; y++)
    {
        for (x = 0; x < map->width; x++)
        {
            uint8_t tile_id;
            gfx_sprite_t *sprite;

            tile_id = tilemap_get(map, x, y);
            sprite = tileset[tile_id].sprite;

            /*
             * Don't attempt to draw an unassigned tile.
             */
            if (sprite == NULL)
                continue;

            gfx_Sprite(
                sprite,
                screen_x + x * TILE_SIZE,
                screen_y + y * TILE_SIZE
            );
        }
    }
}


void tilemap_draw_region(
    const TileMap *map,
    const Tile *tileset,
    uint16_t start_x,
    uint16_t start_y,
    uint16_t width,
    uint16_t height,
    int16_t screen_x,
    int16_t screen_y
)
{
    uint16_t x;
    uint16_t y;

    /*
     * Don't start outside the map.
     */
    if (start_x >= map->width || start_y >= map->height)
        return;

    /*
     * Clip the requested region to the map.
     */
    if (start_x + width > map->width)
        width = map->width - start_x;

    if (start_y + height > map->height)
        height = map->height - start_y;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            uint8_t tile_id;
            gfx_sprite_t *sprite;

            tile_id = tilemap_get(
                map,
                start_x + x,
                start_y + y
            );

            sprite = tileset[tile_id].sprite;

            if (sprite == NULL)
                continue;

            gfx_Sprite(
                sprite,
                screen_x + x * TILE_SIZE,
                screen_y + y * TILE_SIZE
            );
        }
    }
}

void tilemap_draw_column(
    const TileMap *map,
    const Tile *tileset,
    uint16_t world_x,
    uint16_t world_y,
    uint16_t height,
    int16_t screen_x,
    int16_t screen_y
)
{
    uint16_t y;

    for (y = 0; y < height; y++)
    {
        uint8_t tile_id;
        gfx_sprite_t *sprite;

        tile_id = tilemap_get(
            map,
            world_x,
            world_y + y
        );

        sprite = tileset[tile_id].sprite;

        if (sprite == NULL)
            continue;

        gfx_Sprite(
            sprite,
            screen_x,
            screen_y + y * TILE_SIZE
        );
    }
}

void tilemap_draw_row(
    const TileMap *map,
    const Tile *tileset,
    uint16_t world_x,
    uint16_t world_y,
    uint16_t width,
    int16_t screen_x,
    int16_t screen_y
)
{
    uint16_t x;

    for (x = 0; x < width; x++)
    {
        uint8_t tile_id;
        gfx_sprite_t *sprite;

        tile_id = tilemap_get(
            map,
            world_x + x,
            world_y
        );

        sprite = tileset[tile_id].sprite;

        if (sprite == NULL)
            continue;

        gfx_Sprite(
            sprite,
            screen_x + x * TILE_SIZE,
            screen_y
        );
    }
}