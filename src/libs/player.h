#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#include "tiles.h"
#include "camera.h"
#include "inventory.h"

/*
 * Player movement speed in pixels per frame.
 */
#define PLAYER_SPEED 4

typedef enum
{
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} Direction;

typedef struct
{
    /* Grid position */
    uint16_t tile_x;
    uint16_t tile_y;

    /* Smooth visual position in world pixels */
    int16_t visual_x;
    int16_t visual_y;

    /* Target position in world pixels */
    int16_t target_x;
    int16_t target_y;

    /* Position on the screen */
    int16_t screen_x;
    int16_t screen_y;

    /* Previous screen position */
    int16_t old_screen_x;
    int16_t old_screen_y;

    /* Whether the player is currently moving */
    uint8_t moving;

    /* The direction the player is looking */
    Direction facing;

    /* Player's inventory & other information */
    Inventory inventory;

} Player;


/*
 * Initialize a player at a tile position.
 */
void player_init(
    Player *player,
    uint16_t tile_x,
    uint16_t tile_y
);


/*
 * Check whether a tile can be entered.
 *
 * Tile 0 = walkable.
 * Every other tile = collision.
 */
uint8_t player_can_move_to(
    const TileMap *world,
    uint16_t x,
    uint16_t y
);

void player_warp(
    Player *player,
    uint16_t tile_x,
    uint16_t tile_y
);


/*
 * Read the keyboard and start player movement
 * if possible.
 */
void player_handle_input(
    Player *player,
    const TileMap *world
);


/*
 * Move the player's visual position toward
 * its target position.
 */
void player_update(
    Player *player
);


/*
 * Update the player's screen position based
 * on the camera's current position.
 */
void player_update_screen_position(
    Player *player,
    const Camera *camera
);


/*
 * Returns nonzero if the player's screen position
 * changed since the previous frame.
 */
uint8_t player_screen_changed(
    const Player *player
);


/*
 * Save the current screen position as the old
 * position for the next frame.
 */
void player_commit_screen_position(
    Player *player
);

void get_facing_tile(
    const Player *player,
    uint16_t *x,
    uint16_t *y
);

#endif