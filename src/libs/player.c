#include <keypadc.h>

#include "player.h"
#include "inventory.h"

static const uint8_t tile_walkable[MAX_TILES] =
{
    [2]  = 1,
    [7]  = 1,
    [8]  = 1,
    [9]  = 1,
    [10]  = 1,
    [11] = 1,
    [12] = 1,
    [36] = 1
};


void player_init(
    Player *player,
    uint16_t tile_x,
    uint16_t tile_y
)
{
    player->tile_x = tile_x;
    player->tile_y = tile_y;

    player->visual_x = tile_x * TILE_SIZE;
    player->visual_y = tile_y * TILE_SIZE;

    player->target_x = player->visual_x;
    player->target_y = player->visual_y;

    player->screen_x =
        player->visual_x + 2;

    player->screen_y =
        player->visual_y;

    player->old_screen_x =
        player->screen_x;

    player->old_screen_y =
        player->screen_y;

    player->moving = 0;

    inventory_init(&player->inventory);
}

uint8_t player_can_move_to(
    const TileMap *world,
    uint16_t x,
    uint16_t y
)
{
    if (x >= world->width || y >= world->height)
        return 0;

    return tile_walkable[tilemap_get(world, x, y)] != 0;
}


void player_warp(
    Player *player,
    uint16_t tile_x,
    uint16_t tile_y
)
{
    player->tile_x = tile_x;
    player->tile_y = tile_y;

    player->visual_x = tile_x * TILE_SIZE;
    player->visual_y = tile_y * TILE_SIZE;

    player->target_x = player->visual_x;
    player->target_y = player->visual_y;

    player->moving = 0;
}

void player_handle_input(
    Player *player,
    const TileMap *world
)
{
    int8_t move_x = 0;
    int8_t move_y = 0;

    /*
     * Don't accept another movement while
     * the player is already moving.
     */
    if (player->moving)
        return;

    /*
     * Match the original movement priority:
     *
     * Right
     * Left
     * Down
     * Up
     */
    if (kb_IsDown(kb_KeyRight)) {
        move_x = 1;
        player->facing = DIRECTION_RIGHT;
    }
    else if (kb_IsDown(kb_KeyLeft)) {
        move_x = -1;
        player->facing = DIRECTION_LEFT;
    }
    else if (kb_IsDown(kb_KeyDown)) {
        move_y = 1;
        player->facing = DIRECTION_DOWN;
    }
    else if (kb_IsDown(kb_KeyUp)) {
        move_y = -1;
        player->facing = DIRECTION_UP;
    }

    if (!move_x && !move_y)
        return;

    /*
     * Calculate the destination tile.
     */
    int16_t target_x = player->tile_x + move_x;
    int16_t target_y = player->tile_y + move_y;

    /*
     * Prevent negative coordinates.
     */
    if (target_x < 0 || target_y < 0)
        return;

    /*
     * Check collision.
     */
    if (!player_can_move_to(
            world,
            target_x,
            target_y))
    {
        return;
    }

    /*
     * Update the logical grid position.
     */
    player->tile_x = target_x;
    player->tile_y = target_y;

    /*
     * Set the smooth movement target.
     */
    player->target_x = target_x * TILE_SIZE;
    player->target_y = target_y * TILE_SIZE;

    player->moving = 1;
}


void player_update(
    Player *player
)
{
    /*
     * Move visual X toward target X.
     */
    if (player->visual_x < player->target_x)
    {
        player->visual_x += PLAYER_SPEED;

        if (player->visual_x > player->target_x)
            player->visual_x = player->target_x;
    }
    else if (player->visual_x > player->target_x)
    {
        player->visual_x -= PLAYER_SPEED;

        if (player->visual_x < player->target_x)
            player->visual_x = player->target_x;
    }

    /*
     * Move visual Y toward target Y.
     */
    if (player->visual_y < player->target_y)
    {
        player->visual_y += PLAYER_SPEED;

        if (player->visual_y > player->target_y)
            player->visual_y = player->target_y;
    }
    else if (player->visual_y > player->target_y)
    {
        player->visual_y -= PLAYER_SPEED;

        if (player->visual_y < player->target_y)
            player->visual_y = player->target_y;
    }

    /*
     * Movement is finished once the visual position
     * reaches the target.
     */
    if (player->visual_x == player->target_x &&
        player->visual_y == player->target_y)
    {
        player->moving = 0;
    }
}


void player_update_screen_position(
    Player *player,
    const Camera *camera
)
{
    int16_t camera_pixel_x =
        camera->x * TILE_SIZE + camera->pixel_x;

    int16_t camera_pixel_y =
        camera->y * TILE_SIZE + camera->pixel_y;

    /*
     * Keep the original +2 X offset.
     */
    player->screen_x =
        player->visual_x - camera_pixel_x + 2;

    player->screen_y =
        player->visual_y - camera_pixel_y;
}


uint8_t player_screen_changed(
    const Player *player
)
{
    return (
        player->screen_x != player->old_screen_x ||
        player->screen_y != player->old_screen_y
    );
}


void player_commit_screen_position(
    Player *player
)
{
    player->old_screen_x = player->screen_x;
    player->old_screen_y = player->screen_y;
}

void get_facing_tile(
    const Player *player,
    uint16_t *x,
    uint16_t *y
)
{
    *x = player->tile_x;
    *y = player->tile_y;

    switch (player->facing)
    {
        case DIRECTION_UP:
            if (*y > 0)
                (*y)--;
            break;

        case DIRECTION_DOWN:
            (*y)++;
            break;

        case DIRECTION_LEFT:
            if (*x > 0)
                (*x)--;
            break;

        case DIRECTION_RIGHT:
            (*x)++;
            break;
    }
}