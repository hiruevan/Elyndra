#ifndef RENDERING_H
#define RENDERING_H

#include <stdint.h>
#include <graphics.h>

#include "tiles.h"
#include "camera.h"
#include "player.h"


typedef struct
{
    Sprite *player_background;

    int16_t old_camera_x;
    int16_t old_camera_y;

} Renderer;


void rendering_init(
    Renderer *renderer,
    const Camera *camera
);


void rendering_draw_initial(
    Renderer *renderer,
    const TileMap *world,
    Tile *tileset,
    const Camera *camera,
    Player *player
);


void rendering_update(
    Renderer *renderer,
    const TileMap *world,
    Tile *tileset,
    const Camera *camera,
    Player *player
);

void rendering_resync(
    Renderer *renderer,
    const Camera *camera
);

#endif