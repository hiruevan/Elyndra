#include <keypadc.h>

#include "pause_screen.h"
#include "game_state.h"
#include "inventory_screen.h"
#include "libs/input.h"

// Key states
static uint8_t prev_action_key = 0;
static uint8_t prev_up = 0;
static uint8_t prev_down = 0;

bool pause_screen(Renderer *renderer, Camera *camera, Player *player, TileMap *current_map, GameState *current_state, Tile *tileset)
{
    static uint8_t selected = 0;

    const char *options[] = {
        "Continue",
        "Inventory",
        "Status",
        "Save",
        "Quit Game"
    };

    const uint8_t option_count = 5;

    // Draw background
    gfx_FillScreen(1);
    gfx_SetTextFGColor(223);

    // Title
    gfx_SetTextScale(2, 2);
    gfx_SetTextXY(120, 35);
    gfx_PrintString("PAUSED");

    // Options
    gfx_SetTextScale(1, 1);

    for (uint8_t i = 0; i < option_count; i++)
    {
        uint8_t y = 85 + i * 30;

        if (i == selected)
        {
            gfx_SetTextFGColor(250);
            gfx_PrintStringXY("> ", 85, y);
        }
        else
        {
            gfx_SetTextFGColor(223);
            gfx_PrintStringXY("  ", 85, y);
        }

        gfx_PrintStringXY(options[i], 105, y);
    }

    // Input
    uint8_t up = kb_IsDown(kb_KeyUp);
    uint8_t down = kb_IsDown(kb_KeyDown);
    uint8_t action = kb_IsDown(kb_Key2nd);

    if (key_just_pressed(prev_up, up))
    {
        if (selected > 0)
            selected--;
    }

    if (key_just_pressed(prev_down, down))
    {
        if (selected < option_count - 1)
            selected++;
    }

    if (key_just_pressed(prev_action_key, action))
    {
        switch (selected)
        {
            case 0: // Continue
                *current_state = GAME_EXPLORATION;
                rendering_draw_initial(renderer, current_map, tileset, camera, player);
                break;

            case 1: // Inventory
            inventory_screen_open();
                *current_state = GAME_INVENTORY;
                break;
            
            case 2: // Status
                *current_state = GAME_STATUS_MENU;
                break;

            case 3: // Save
                // TODO: Implement save system -------------------------------------------------------------------------------
                break;

            case 4: // Quit Game
                return false;
        }
    }

    prev_up = up;
    prev_down = down;
    prev_action_key = action;

    gfx_SwapDraw();
    return true;
}