#include <keypadc.h>
#include <graphx.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "pause_screen.h"
#include "game_state.h"
#include "inventory_screen.h"
#include "../libs/save.h"
#include "../libs/area.h"
#include "../libs/input.h"
#include "../interactions/interactables.h"

#define SAVE_MSG_FRAMES 60

bool pause_screen(Renderer *renderer, Camera *camera, Player *player, TileMap *current_map, AreaId current_area, GameState *current_state, Tile *tileset,
uint8_t *prev_up, uint8_t *prev_down, uint8_t *prev_action_key, uint8_t *prev_back_key, uint8_t *prev_clear_key,
uint8_t save_slot, GameSave *save)
{
    static uint8_t selected = 0;
    static uint8_t msg_timer = 0;
    static bool msg_ok = false;

    const char *options[] = {
        "Continue",
        "Inventory",
        "Save",
        "Quit Game"
    };

    const uint8_t option_count = 4;

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
        uint8_t y = (uint8_t)(85 + i * 30);

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

    // Save feedback message
    if (msg_timer > 0)
    {
        gfx_SetTextFGColor(223);
        gfx_PrintStringXY(msg_ok ? "Game saved!" : "Save failed!", 105, 210);
        msg_timer--;
    }

    // Input
    uint8_t up     = kb_IsDown(kb_KeyUp);
    uint8_t down   = kb_IsDown(kb_KeyDown);
    uint8_t back   = kb_IsDown(kb_KeyAlpha);
    uint8_t clear  = kb_IsDown(kb_KeyClear);
    uint8_t action = kb_IsDown(kb_Key2nd);

    if (key_just_pressed(*prev_up, up))
    {
        if (selected > 0)
            selected--;
    }

    if (key_just_pressed(*prev_down, down))
    {
        if (selected < option_count - 1)
            selected++;
    }

    if (key_just_pressed(*prev_action_key, action))
    {
        switch (selected)
        {
            case 0: // Continue
                msg_timer = 0;
                *current_state = GAME_EXPLORATION;
                rendering_draw_initial(renderer, current_map, tileset, camera, player);
                break;

            case 1: // Inventory
                msg_timer = 0;
                *current_state = GAME_INVENTORY;
                break;

            case 2: // Save
                if (save_slot >= 0 && save != NULL)
                {
                    save_capture(save, player, current_area, area_stack, area_stack_top, interactables, INTERACTABLE_COUNT);
                    msg_ok = save_write((uint8_t)save_slot, save);
                }
                else
                {
                    msg_ok = false;
                }
                msg_timer = SAVE_MSG_FRAMES;
                break;

            case 3: // Save & quit Game
                if (save_slot >= 0 && save != NULL)
                {
                    save_capture(save, player, current_area, area_stack, area_stack_top, interactables, INTERACTABLE_COUNT);
                    save_write((uint8_t)save_slot, save);
                }
                msg_timer = 0;
                return false;
        }
    }

    if (key_just_pressed(*prev_back_key, back) || key_just_pressed(*prev_clear_key, clear))
    {
        msg_timer = 0;
        *current_state = GAME_EXPLORATION;
        rendering_draw_initial(renderer, current_map, tileset, camera, player);
    }

    *prev_up = up;
    *prev_down = down;
    *prev_clear_key = clear;
    *prev_back_key = back;
    *prev_action_key = action;

    gfx_SwapDraw();
    return true;
}