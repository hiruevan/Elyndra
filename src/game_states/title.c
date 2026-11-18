#include "title.h"
#include "../libs/save.h"
#include "libs/rendering.h"
#include "libs/player.h"
#include "libs/tiles.h"
#include "../interactions/interactables.h"
#include "game_state.h"

#include <graphics.h>
#include <files.h>
#include <string.h>
#include <stdio.h>

#include <input.h>

#define TEXT_COLOR 223
#define BG_COLOR   1

/* Max characters the player can type for a save name (must be <= NAME_LEN) */
#define NAME_PROMPT_LEN 5

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

// Blocks until nothing is pressed. Scans while waiting, because KeyDown()
// only changes when Input_Scan() runs.
static void wait_release(void)
{
    do {
        Input_Scan();
    } while (KeyDown(INPUT.AnyKey));
}

/* ------------------------------------------------------------------ */
/* Save select                                                         */
/* ------------------------------------------------------------------ */

// Returns true if the player confirmed deleting the save
static bool confirm_delete(const char *name)
{
    char buff[100];

    wait_release();

    for (;;) {
        FillScreen(BG_COLOR);
        SetTextFGColor(TEXT_COLOR);

        SetTextScale(2, 2);
        SetTextXY(60, 60);
        PrintString("DELETE SAVE?");

        SetTextScale(1, 1);
        SetTextXY(60, 110);
        PrintString(name);

        SetTextXY(60, 140);
        PrintString("This cannot be undone.");

        SetTextXY(60, 180);
        sprintf(buff, "%s: delete", INPUT.ActionKey.name);
        PrintString(buff);

        SetTextXY(60, 195);
        sprintf(buff, "%s / %s: cancel", INPUT.ClearKey.name, INPUT.BackKey.name);
        PrintString(buff);
        SwapDraw();

        Input_Scan();

        if (KeyJustPressed(INPUT.ActionKey)) {
            return true;
        }
        if (KeyJustPressed(INPUT.ClearKey) || KeyJustPressed(INPUT.BackKey)) {
            return false;
        }
    }
}

// Returns the chosen slot (0..MAX_SAVES-1) with out_save filled in,
// or -1 if the player backed out
int8_t select_save_logic(GameSave *out_save)
{
    GameSave slots[MAX_SAVES];
    bool used[MAX_SAVES];
    uint8_t sel = 0;
    char buff[100];

    for (uint8_t i = 0; i < MAX_SAVES; i++) {
        used[i] = save_read(i, &slots[i]);
    }

    // Ignore anything still held from the previous screen
    wait_release();

    for (;;) {
        // ---- Draw ----
        FillScreen(BG_COLOR);
        SetTextFGColor(TEXT_COLOR);

        SetTextScale(2, 2);
        SetTextXY(70, 30);
        PrintString("SELECT SAVE");

        SetTextScale(1, 1);
        for (uint8_t i = 0; i < MAX_SAVES; i++) {
            uint8_t y = 80 + i * 30;

            SetTextXY(60, y);
            PrintString(i == sel ? ">" : " ");

            SetTextXY(75, y);
            PrintUInt(i + 1, 1);
            PrintString(". ");

            if (used[i]) {
                PrintString(slots[i].name);
                PrintString("  ");
                PrintString(get_area_name(slots[i].area));
            } else {
                PrintString("- New Game -");
            }
        }

        SetTextXY(60, 190);
        sprintf(buff, "%s: select   %s: back", INPUT.ActionKey.name, INPUT.BackKey.name);
        PrintString(buff);
        SetTextXY(60, 205);
        sprintf(buff, "%s: delete save", INPUT.ClearKey.name);
        PrintString(buff);
        SwapDraw();

        // ---- Input ----
        Input_Scan();

        if (KeyJustPressed(INPUT.BackKey)) {
            return -1;
        }

        if (KeyJustPressed(INPUT.UpKey)   && sel > 0)             sel--;
        if (KeyJustPressed(INPUT.DownKey) && sel < MAX_SAVES - 1) sel++;

        if (KeyJustPressed(INPUT.ClearKey)) {
            if (used[sel] && confirm_delete(slots[sel].name)) {
                if (save_delete(sel)) {
                    used[sel] = false;
                    memset(&slots[sel], 0, sizeof(GameSave));
                }
            }

            wait_release();
            continue;
        }

        if (KeyJustPressed(INPUT.ActionKey)) {
            if (used[sel]) {
                *out_save = slots[sel];
                return (int8_t)sel;
            }

            // Empty slot: create a new game here
            const char *result = TextPrompt("Enter your name:", NAME_PROMPT_LEN);

            if (result && result[0] != '\0') {
                char name[NAME_LEN + 1];
                strncpy(name, result, NAME_LEN);
                name[NAME_LEN] = '\0';

                memset(out_save, 0, sizeof(GameSave));

                strcpy(out_save->name, name);

                out_save->player_x = START_X;
                out_save->player_y = START_Y;

                // Initial party
                PartyMember starting_party[] = {
                    {
                        .level    = 1,
                        .hp       = 20, .max_hp = 20,
                        .mp       = 8,  .max_mp = 8,
                        .exp      = 0,  .exp_next = 10,
                        .attack   = 12,
                        .defense  = 16,
                        .speed    = 10,
                        .weapon   = ITEM_RUSTY_SWORD,
                        .armor    = ITEM_CLOTH,
                    }
                };

                strncpy(starting_party[0].name, name, sizeof(starting_party[0].name) - 1);

                out_save->party_count = 1;
                memcpy(out_save->party, starting_party, sizeof(starting_party));

                out_save->area = AREA_OVERWORLD;

                if (save_write(sel, out_save)) {
                    return (int8_t)sel;
                }
            }

            // Cancelled, empty name, or save failed: back to the list
            wait_release();
            continue;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Title                                                               */
/* ------------------------------------------------------------------ */

// Note: Input_Scan() must be called once per frame by the main loop,
// before this function runs.
bool title_logic(Renderer *renderer, Camera *camera, Player *player, GameState *current_state, TileMap *current_map, AreaId *current_area, Tile *tileset, uint8_t *save_slot, GameSave *save)
{
    char buff[100];

    TextPromptSetColors(TEXT_COLOR, BG_COLOR);

    // Exit game
    if (KeyJustPressed(INPUT.ClearKey))
        return false;

    // Clear the screen
    FillScreen(BG_COLOR);
    SetTextFGColor(TEXT_COLOR);

    // Title
    SetTextScale(2, 2);
    SetTextXY(100, 60);
    PrintString("ELYNDRA");

    // Subtitle
    SetTextScale(1, 1);
    SetTextXY(95, 100);
    PrintString("A TI-84 Adventure");

    // Start prompt
    SetTextXY(95, 180);
    sprintf(buff, "Press %s to Start", INPUT.ActionKey.name);
    PrintString(buff);

    // Check for start
    if (KeyJustPressed(INPUT.ActionKey)) {
        GameSave chosen;
        int8_t slot = select_save_logic(&chosen);

        if (slot >= 0) {
            *save_slot = slot;
            *save = chosen;

            load_save(&chosen, player, camera, current_area, area_stack, &area_stack_top, current_map, interactables, INTERACTABLE_COUNT);

            *current_state = GAME_EXPLORATION;

            // clean up rendering buffers
            rendering_resync(renderer, camera);
            rendering_draw_initial(renderer, current_map, tileset, camera, player);
            SwapDraw();

            // Draw first frame
            rendering_draw_initial(renderer, current_map, tileset, camera, player);
        }
    }

    SwapDraw();

    return true;
}