#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>

#include "tiles.h"

typedef struct
{
    int16_t x;
    int16_t y;

    int16_t pixel_x;
    int16_t pixel_y;

} Camera;


/*
 * Initialize the camera.
 *
 * x and y are world tile coordinates.
 */
void camera_init(
    Camera *camera,
    int16_t x,
    int16_t y
);

void camera_snap_to(
    Camera *camera,
    int16_t center_x,
    int16_t center_y,
    const TileMap *map
);


/*
 * Move the camera by a number of tiles.
 */
void camera_move(
    Camera *camera,
    int16_t dx,
    int16_t dy,
    const TileMap *map
);

#endif