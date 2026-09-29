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

// Game States
#include "game_states/game_state.h"
#include "game_states/title.h"
#include "game_states/exploration.h"
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


// Player start pos
#define START_X 10
#define START_Y 10

// Gamestate
static GameState current_state = GAME_TITLE;

// Area & tilemap
static TileMap current_map;
static AreaId current_area = AREA_OVERWORLD;

// Action key
static uint8_t prev_action_key = 0;

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
    load_area(current_area, &current_map);

    // Create Camera
    Camera camera;
    camera_init(&camera, 0, 0);

    // Create Player
    Player player;
    player_init(&player, START_X, START_Y);

    // Snap camera to init player
    camera_snap_to(&camera, player.screen_x, player.screen_y, &current_map);

    // Create renderer
    Renderer renderer;
    rendering_init(&renderer, &camera);

    // Main Loop
    while (1)
    {
        kb_Scan();

        // Action key
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
            exploration_logic(&renderer, &camera, &player, action_key_down, &prev_action_key, &current_state, &current_map, tileset, &current_area);
        else if (current_state == GAME_TITLE)
            title_logic(&renderer, &camera, &player, action_key_down, &current_state, &current_map, tileset);
        else if (current_state == GAME_PAUSE_MENU) {
            if (!pause_screen(&renderer, &camera, &player, &current_map, &current_state, tileset))
                break;
        }
        else if (current_state == GAME_INVENTORY)
            if (!inventory_screen(&player))
                current_state = GAME_PAUSE_MENU;
            }

    // Cleanup
    gfx_End();

    return 0;
}