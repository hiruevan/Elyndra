#include <graphics.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <input.h>

#include "pause_screen.h"
#include "game_state.h"
#include "inventory_screen.h"
#include "../libs/save.h"
#include "../libs/area.h"
#include "../interactions/interactables.h"

#define SAVE_MSG_FRAMES 60

bool pause_screen(Renderer *renderer, Camera *camera, Player *player, TileMap *current_map, AreaId current_area, GameState *current_state, Tile *tileset,
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
    FillScreen(1);
    SetTextFGColor(223);

    // Title
    SetTextScale(2, 2);
    SetTextXY(120, 35);
    PrintString("PAUSED");

    // Options
    SetTextScale(1, 1);

    for (uint8_t i = 0; i < option_count; i++)
    {
        uint8_t y = (uint8_t)(85 + i * 30);

        if (i == selected)
        {
            SetTextFGColor(250);
            PrintStringXY("> ", 85, y);
        }
        else
        {
            SetTextFGColor(223);
            PrintStringXY("  ", 85, y);
        }

        PrintStringXY(options[i], 105, y);
    }

    // Save feedback message
    if (msg_timer > 0)
    {
        SetTextFGColor(223);
        PrintStringXY(msg_ok ? "Game saved!" : "Save failed!", 105, 210);
        msg_timer--;
    }

    if (KeyJustPressed(INPUT.UpKey))
    {
        if (selected > 0)
            selected--;
    }

    if (KeyJustPressed(INPUT.DownKey))
    {
        if (selected < option_count - 1)
            selected++;
    }

    if (KeyJustPressed(INPUT.ActionKey))
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

    if (KeyJustPressed(INPUT.BackKey) || KeyJustPressed(INPUT.ClearKey))
    {
        msg_timer = 0;
        *current_state = GAME_EXPLORATION;
        rendering_draw_initial(renderer, current_map, tileset, camera, player);
    }

    SwapDraw();
    return true;
}