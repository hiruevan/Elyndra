#include "camera.h"


void camera_init(
    Camera *camera,
    int16_t x,
    int16_t y
)
{
    camera->x = x;
    camera->y = y;

    camera->pixel_x = 0;
    camera->pixel_y = 0;
}


void camera_move(
    Camera *camera,
    int16_t dx,
    int16_t dy,
    const TileMap *map
)
{
    int16_t new_x;
    int16_t new_y;

    new_x =
        camera->x * TILE_SIZE +
        camera->pixel_x +
        dx;

    new_y =
        camera->y * TILE_SIZE +
        camera->pixel_y +
        dy;

    if (new_x < 0)
        new_x = 0;

    if (new_y < 0)
        new_y = 0;

    if (new_x >
        (int16_t)(map->width * TILE_SIZE -
                  MAP_VIEW_WIDTH * TILE_SIZE))
    {
        new_x =
            map->width * TILE_SIZE -
            MAP_VIEW_WIDTH * TILE_SIZE;
    }

    if (new_y >
        (int16_t)(map->height * TILE_SIZE -
                  MAP_VIEW_HEIGHT * TILE_SIZE))
    {
        new_y =
            map->height * TILE_SIZE -
            MAP_VIEW_HEIGHT * TILE_SIZE;
    }

    camera->x = new_x / TILE_SIZE;
    camera->pixel_x = new_x % TILE_SIZE;

    camera->y = new_y / TILE_SIZE;
    camera->pixel_y = new_y % TILE_SIZE;
}

void camera_snap_to(
    Camera *camera,
    int16_t center_x,
    int16_t center_y,
    const TileMap *map
)
{
    camera_init(camera, 0, 0);

    camera_move(
        camera,
        center_x - 152,
        center_y - 112,
        map
    );
}