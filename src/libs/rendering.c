#include <graphics.h>

#include "rendering.h"
#include "../sprites/player_sprite.h"


void rendering_init(
    Renderer *renderer,
    const Camera *camera
)
{
    renderer->old_camera_x =
        camera->x * TILE_SIZE + camera->pixel_x;

    renderer->old_camera_y =
        camera->y * TILE_SIZE + camera->pixel_y;

    renderer->player_background =
        MallocSprite(
            PLAYER_WIDTH,
            PLAYER_HEIGHT
        );

    /*
     * Use the drawing buffer.
     */
    SetDrawBuffer();
}


void rendering_draw_initial(
    Renderer *renderer,
    const TileMap *world,
    Tile *tileset,
    const Camera *camera,
    Player *player
)
{
    /*
     * Draw initial world.
     */
    tilemap_draw_region(
        world,
        tileset,
        camera->x,
        camera->y,
        MAP_VIEW_WIDTH + 1,
        MAP_VIEW_HEIGHT + 1,
        -camera->pixel_x,
        -camera->pixel_y
    );

    /*
     * Save the background underneath the player.
     */
    GetSprite(
        renderer->player_background,
        player->screen_x,
        player->screen_y
    );

    /*
     * Draw player.
     */
    DrawTransparentSprite(
        player_sprite,
        player->screen_x,
        player->screen_y
    );
}


void rendering_update(
    Renderer *renderer,
    const TileMap *world,
    Tile *tileset,
    const Camera *camera,
    Player *player
)
{
    /*
     * Current camera position in pixels.
     */
    int16_t camera_pixel_x =
        camera->x * TILE_SIZE + camera->pixel_x;

    int16_t camera_pixel_y =
        camera->y * TILE_SIZE + camera->pixel_y;

    /*
     * Calculate camera movement.
     */
    int16_t dx =
        camera_pixel_x - renderer->old_camera_x;

    int16_t dy =
        camera_pixel_y - renderer->old_camera_y;

    /*
     * Calculate player movement.
     */
    int16_t player_dx =
        player->screen_x - player->old_screen_x;

    int16_t player_dy =
        player->screen_y - player->old_screen_y;

    /*
     * Nothing changed.
     */
    if (dx == 0 &&
        dy == 0 &&
        player_dx == 0 &&
        player_dy == 0)
    {
        return;
    }

    /*
     * Copy the currently visible frame into
     * the drawing buffer.
     */
    BlitScreen();

    /*
     * Remove the player by restoring the background
     * that was underneath them.
     */
    DrawSprite(
        renderer->player_background,
        player->old_screen_x,
        player->old_screen_y
    );

    /*
     * If only the player moved, the camera did not move.
     *
     * The existing framebuffer already contains the
     * correct world, so no tiles need to be shifted
     * or redrawn.
     */
    if (dx == 0 && dy == 0)
    {
        GetSprite(
            renderer->player_background,
            player->screen_x,
            player->screen_y
        );

        DrawTransparentSprite(
            player_sprite,
            player->screen_x,
            player->screen_y
        );

        SwapDraw();

        player_commit_screen_position(player);

        return;
    }


    /*
     * Horizontal scrolling
     */
    if (dx > 0)
    {
        LCDShiftLeft(dx);

        SetClipRegion(
            LCD_WIDTH - dx,
            0,
            LCD_WIDTH,
            LCD_HEIGHT
        );

        if (camera->pixel_x == 0)
        {
            /*
             * Just crossed a tile boundary.
             */
            tilemap_draw_column(
                world,
                tileset,
                camera->x + MAP_VIEW_WIDTH - 1,
                camera->y,
                MAP_VIEW_HEIGHT + 1,
                LCD_WIDTH - TILE_SIZE,
                -camera->pixel_y
            );
        }
        else
        {
            /*
             * Partway through the next tile.
             */
            tilemap_draw_column(
                world,
                tileset,
                camera->x + MAP_VIEW_WIDTH,
                camera->y,
                MAP_VIEW_HEIGHT + 1,
                LCD_WIDTH - camera->pixel_x,
                -camera->pixel_y
            );
        }
    }
    else if (dx < 0)
    {
        LCDShiftRight(-dx);

        SetClipRegion(
            0,
            0,
            -dx,
            LCD_HEIGHT
        );

        tilemap_draw_column(
            world,
            tileset,
            camera->x,
            camera->y,
            MAP_VIEW_HEIGHT + 1,
            -camera->pixel_x,
            -camera->pixel_y
        );
    }


    /*
     * Vertical scrolling
     */
    if (dy > 0)
    {
        LCDShiftUp(dy);

        SetClipRegion(
            0,
            LCD_HEIGHT - dy,
            LCD_WIDTH,
            LCD_HEIGHT
        );

        if (camera->pixel_y == 0)
        {
            /*
             * Just crossed a tile boundary.
             */
            tilemap_draw_row(
                world,
                tileset,
                camera->x,
                camera->y + MAP_VIEW_HEIGHT - 1,
                MAP_VIEW_WIDTH + 1,
                -camera->pixel_x,
                LCD_HEIGHT - TILE_SIZE
            );
        }
        else
        {
            /*
             * Partway through the next tile.
             */
            tilemap_draw_row(
                world,
                tileset,
                camera->x,
                camera->y + MAP_VIEW_HEIGHT,
                MAP_VIEW_WIDTH + 1,
                -camera->pixel_x,
                LCD_HEIGHT - camera->pixel_y
            );
        }
    }
    else if (dy < 0)
    {
        LCDShiftDown(-dy);

        SetClipRegion(
            0,
            0,
            LCD_WIDTH,
            -dy
        );

        tilemap_draw_row(
            world,
            tileset,
            camera->x,
            camera->y,
            MAP_VIEW_WIDTH + 1,
            -camera->pixel_x,
            -camera->pixel_y
        );
    }


    /*
     * Reset clipping.
     */
    SetClipRegion(
        0,
        0,
        LCD_WIDTH,
        LCD_HEIGHT
    );


    /*
     * Save the new background underneath the player.
     */
    GetSprite(
        renderer->player_background,
        player->screen_x,
        player->screen_y
    );

    /*
     * Draw player on top of world.
     */
    DrawTransparentSprite(
        player_sprite,
        player->screen_x,
        player->screen_y
    );


    /*
     * Show completed frame.
     */
    SwapDraw();


    /*
     * Remember camera position.
     */
    renderer->old_camera_x = camera_pixel_x;
    renderer->old_camera_y = camera_pixel_y;

    /*
     * Remember player position.
     */
    player_commit_screen_position(player);
}

void rendering_resync(
    Renderer *renderer,
    const Camera *camera
)
{
    renderer->old_camera_x =
        camera->x * TILE_SIZE + camera->pixel_x;

    renderer->old_camera_y =
        camera->y * TILE_SIZE + camera->pixel_y;
}