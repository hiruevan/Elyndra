// General
#include <stdio.h>

// Calculator C-libs
#include <graphx.h>
#include <keypadc.h>

// My own libs
#include "libs/sprite_utils.h"
#include "libs/tiles.h"
#include "libs/camera.h"
#include "libs/area.h"
#include "libs/interactable.h"
#include "libs/player.h"
#include "libs/rendering.h"
#include "libs/text_engine.h"
#include "libs/dialogue.h"
#include "libs/inventory.h"
#include "libs/input.h"
#include "libs/save.h"

// Game States
#include "game_states/game_state.h"
#include "game_states/title.h"
#include "game_states/exploration.h"
#include "game_states/battle.h"
#include "game_states/pause_screen.h"
#include "game_states/inventory_screen.h"

// Maps
#include "maps/world_map.h"
#include "maps/town_map.h"
#include "maps/test_map.h"
#include "maps/test_house_map.h"

// Tileset
#include "tiles/tileset.h"

// Sprites
#include "sprites/player_sprite.h"
#include "tiles/tile_sprites.h"

// Interactions
#include "interactions/interactables.h"
#include "interactions/items.h"

// Save
static GameSave current_save;
static uint8_t current_save_slot;

// Gamestate
static GameState current_state = GAME_TITLE;

// Area & tilemap
static TileMap current_map;
static AreaId current_area = AREA_OVERWORLD;

// --- Buttons ---
// A & B
static uint8_t prev_action_key = 0;
static uint8_t prev_cancel_key = 0;

// Pause & Inventory
static uint8_t prev_clear_key = 0;
static uint8_t prev_inv_key = 0;

// D-pad
static uint8_t prev_up = 0;
static uint8_t prev_down = 0;
static uint8_t prev_left = 0;
static uint8_t prev_right = 0;

// Player, camera, & renderer
static Player player;
static Camera camera;
static Renderer renderer;

// Main function
int main(void)
{
    // Grafix setup
    gfx_Begin();
    setup_xlibc_palette();

    // Load tileset
    loadTileset();
    tiles_set(tileset, 40, player_sprite);

    // Create world
    tilemap_init(&current_map, 128, 128, 0);

    // Camera
    camera_init(&camera, 0, 0);

    // Renderer
    rendering_init(&renderer, &camera);

    // Main Loop
    while (1)
    {
        // Input
        kb_Scan();

        // Force Exit
        if (kb_IsDown(kb_KeyMode) && kb_IsDown(kb_Key2nd)) {
            gfx_End();
            return 0;
        }

        // Action key & cancel key
        uint8_t action_key_down = kb_IsDown(kb_Key2nd);

        // Textbox draw
        if (textbox_is_active())
        {
            rendering_draw_initial(&renderer, &current_map, tileset, &camera, &player);

            if (key_just_pressed(prev_action_key, action_key_down)) {
                if (dialogue_is_active()) // Dialogue case
                    dialogue_update(action_key_down);
                else // Simple Textbox case
                    textbox_close();
            }
            else
                textbox_draw();     

            gfx_SwapDraw();

            prev_action_key = action_key_down;
            continue;
        }

        if (current_state == GAME_EXPLORATION)
            exploration_logic(&renderer, &camera, &player, &current_state, &current_map, tileset, &current_area, action_key_down, &prev_action_key, &prev_clear_key, &prev_inv_key);
        else if (current_state == GAME_TITLE) {
            if (!title_logic(&renderer, &camera, &player, action_key_down, &current_state, &current_map, &current_area, tileset, &current_save_slot, &current_save))
                break;
        }
        else if (current_state == GAME_BATTLE)
            battle_logic(&current_state, &player, &current_map, tileset, &renderer, &camera);
        else if (current_state == GAME_PAUSE_MENU) {
            if (!pause_screen(&renderer, &camera, &player, &current_map, current_area, &current_state, tileset, &prev_up, &prev_down, &prev_action_key, &prev_cancel_key, &prev_clear_key, current_save_slot, &current_save))
                break;
        }
        else if (current_state == GAME_INVENTORY) {
            if (!inventory_screen(&player, &prev_up, &prev_down, &prev_left, &prev_right, &prev_action_key, &prev_cancel_key, &prev_inv_key, &current_state, &current_map, tileset, &renderer, &camera))
                current_state = GAME_PAUSE_MENU;
        }

    }

    // Save
    save_capture(&current_save, &player, current_area, area_stack, area_stack_top, interactables, INTERACTABLE_COUNT);
    // Cleanup
    gfx_End();

    return 0;
}