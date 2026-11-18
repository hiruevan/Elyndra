// General
#include <stdio.h>

// Calculator C-libs
#include <graphics.h>

// Input
#include <input.h>

// Resources
#include <resources.h>

// My own libs
#include "libs/tiles.h"
#include "libs/camera.h"
#include "libs/area.h"
#include "libs/interactable.h"
#include "libs/player.h"
#include "libs/rendering.h"
#include "libs/text_engine.h"
#include "libs/dialogue.h"
#include "libs/inventory.h"
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

// Player, camera, & renderer
static Player player;
static Camera camera;
static Renderer renderer;

// Main function
int main(void)
{
    // Grafix setup
    DrawBegin();
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
        Input_Scan();

        // Force Exit
        // if (kb_IsDown(kb_KeyMode) && kb_IsDown(kb_Key2nd)) {
        //     DrawEnd();
        //     return 0;
        // }

        // Textbox draw
        if (textbox_is_active())
        {
            rendering_draw_initial(&renderer, &current_map, tileset, &camera, &player);

            if (KeyJustPressed(INPUT.ActionKey)) {
                if (dialogue_is_active()) // Dialogue case
                    dialogue_update(KeyDown(INPUT.ActionKey));
                else // Simple Textbox case
                    textbox_close();
            }
            else
                textbox_draw();     

            SwapDraw();
            continue;
        }

        if (current_state == GAME_EXPLORATION)
            exploration_logic(&renderer, &camera, &player, &current_state, &current_map, tileset, &current_area);
        else if (current_state == GAME_TITLE) {
            if (!title_logic(&renderer, &camera, &player, &current_state, &current_map, &current_area, tileset, &current_save_slot, &current_save))
                break;
        }
        else if (current_state == GAME_BATTLE)
            battle_logic(&current_state, &player, &current_map, tileset, &renderer, &camera);
        else if (current_state == GAME_PAUSE_MENU) {
            if (!pause_screen(&renderer, &camera, &player, &current_map, current_area, &current_state, tileset, current_save_slot, &current_save))
                break;
        }
        else if (current_state == GAME_INVENTORY) {
            if (!inventory_screen(&player, &current_state, &current_map, tileset, &renderer, &camera))
                current_state = GAME_PAUSE_MENU;
        }

    }

    // Save
    save_capture(&current_save, &player, current_area, area_stack, area_stack_top, interactables, INTERACTABLE_COUNT);
    // Cleanup
    DrawEnd();

    return 0;
}