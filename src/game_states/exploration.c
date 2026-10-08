#include <keypadc.h>
#include <tice.h>

#include "exploration.h"
#include "../libs/rendering.h"
#include "../libs/player.h"
#include "../libs/interactable.h"
#include "../libs/input.h"
#include "../libs/area.h"
#include "../interactions/shop_data.h"
#include "pause_screen.h"
#include "game_state.h"

static uint8_t steps_since_encounter = ENCOUNTER_GRACE_STEPS;

// Exploration system
void exploration_logic(Renderer *renderer, Camera *camera, Player *player, GameState *current_state, TileMap *current_map, Tile *tileset, AreaId *current_area, 
    uint8_t action_key_down, uint8_t *prev_action_key, uint8_t *prev_clear_key, uint8_t *prev_inv_key) 
{
    // Inputs
    uint8_t clear_down = kb_IsDown(kb_KeyClear);
    uint8_t inv_down = kb_IsDown(kb_KeyDel);
    uint8_t pause_pressed = key_just_pressed(*prev_clear_key, clear_down);
    uint8_t inv_pressed = key_just_pressed(*prev_inv_key, inv_down);
    uint8_t interacting = key_just_pressed(*prev_action_key, action_key_down);
    
    *prev_clear_key = clear_down;
    *prev_inv_key = inv_down;
    *prev_action_key = action_key_down;

    // Shop
    if (shop_is_active()) 
    {
        shop_screen(player, current_state, current_map, tileset, renderer, camera);
        return;
    }

    // Pause
    if (pause_pressed) {
        *current_state = GAME_PAUSE_MENU;
        return;
    } if (inv_pressed) {
        *current_state = GAME_INVENTORY;
        return;
    }

    // Handle input
    player_handle_input(player, current_map);

    // Update player
    uint8_t was_moving = player->moving;
    player_update(player);

    // Handle interactions
    
    if (interacting && !player->moving)
    {
        uint16_t face_x, face_y;
        get_facing_tile(player, &face_x, &face_y);

        if (face_x < current_map->width && face_y < current_map->height)
        {
            Interactable *interactable =
                find_interactable(*current_area, face_x, face_y);

            if (interactable != NULL)
                handle_interact(interactable, current_map, player);
        }
    }
        

    // Handle portal transitions
    if (was_moving && !player->moving)
    {
        const Portal *portal =
            find_portal(*current_area, player->tile_x, player->tile_y);

        uint8_t at_edge =
            *current_area != AREA_OVERWORLD &&
            (player->tile_x == 0 ||
            player->tile_y == 0 ||
            player->tile_x == current_map->width - 1 ||
            player->tile_y == current_map->height - 1);

        if (portal != NULL || at_edge)
        {
            AreaId  next_area;
            uint16_t next_x, next_y;
            uint8_t have_next = 1;

            if (portal != NULL)
            {
                /* Entering a sub-area: remember where we came from. */
                next_area = portal->to;
                next_x    = portal->to_x;
                next_y    = portal->to_y;

                area_stack_push(portal->from, portal->from_x, portal->from_y);
            }
            else /* walked off the edge of a sub-area */
            {
                have_next = area_stack_pop(&next_area, &next_x, &next_y);
            }

            if (have_next)
            {
                *current_area = next_area;
                load_area(*current_area, current_map);

                player_warp(player, next_x, next_y);
                camera_snap_to(camera, player->visual_x, player->visual_y, current_map);

                player_update_screen_position(player, camera);
                player_commit_screen_position(player);

                rendering_resync(renderer, camera);
                rendering_draw_initial(renderer, current_map, tileset, camera, player);
                gfx_SwapDraw();
            }

            return;
        }
    }

    // Handle random encounters (only on the step that just finished)
    if (was_moving && !player->moving)
    {
        if (steps_since_encounter < ENCOUNTER_GRACE_STEPS)
        {
            steps_since_encounter++;
        }
        else if ((random() % 100) < get_encounter_rate(*current_area))
        {
            steps_since_encounter = 0;
            *current_state = GAME_BATTLE;
            return;
        }
    }

    // Move camera
    int16_t desired_camera_x = player->visual_x - (GFX_LCD_WIDTH - TILE_SIZE) / 2;
    int16_t desired_camera_y = player->visual_y - (GFX_LCD_HEIGHT - TILE_SIZE) / 2;

    int16_t current_camera_x = camera->x * TILE_SIZE + camera->pixel_x;
    int16_t current_camera_y = camera->y * TILE_SIZE + camera->pixel_y;

    camera_move(camera, desired_camera_x - current_camera_x, desired_camera_y - current_camera_y, current_map);

    // Update player screen position
    player_update_screen_position(
        player,
        camera
    );

    // Render
    rendering_update(
        renderer,
        current_map,
        tileset,
        camera,
        player
    );
}