#include "title.h"
#include "../libs/save.h"
#include "libs/rendering.h"
#include "libs/player.h"
#include "libs/tiles.h"
#include "../interactions/interactables.h"
#include "game_state.h"

#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>
#include <string.h>

#define TEXT_COLOR 223
#define BG_COLOR   1

/* ------------------------------------------------------------------ */
/* Key maps                                                            */
/* ------------------------------------------------------------------ */

typedef struct {
    uint8_t group;   // kb_Data[] index
    uint8_t mask;    // bit within that group
    char    ch;      // uppercase letter (or ' ')
} AlphaKey;

// Matches the green letters printed above the keys on the TI-84 CE
static const AlphaKey keys[] = {
    {2, kb_Math,   'A'}, {3, kb_Apps,   'B'}, {4, kb_Prgm,   'C'},
    {2, kb_Recip,  'D'}, {3, kb_Sin,    'E'}, {4, kb_Cos,    'F'},
    {5, kb_Tan,    'G'}, {6, kb_Power,  'H'}, {2, kb_Square, 'I'},
    {3, kb_Comma,  'J'}, {4, kb_LParen, 'K'}, {5, kb_RParen, 'L'},
    {6, kb_Div,    'M'}, {2, kb_Log,    'N'}, {3, kb_7,      'O'},
    {4, kb_8,      'P'}, {5, kb_9,      'Q'}, {6, kb_Mul,    'R'},
    {2, kb_Ln,     'S'}, {3, kb_4,      'T'}, {4, kb_5,      'U'},
    {5, kb_6,      'V'}, {6, kb_Sub,    'W'}, {2, kb_Sto,    'X'},
    {3, kb_1,      'Y'}, {4, kb_2,      'Z'}, {3, kb_0,      ' '},
};
#define KEY_COUNT (sizeof(keys) / sizeof(keys[0]))

// Returns the character for the key currently pressed, or 0 if none
static char scan_char(bool lowercase)
{
    const AlphaKey *table = keys;

    for (size_t i = 0; i < KEY_COUNT; i++) {
        if (kb_Data[table[i].group] & table[i].mask) {
            char c = table[i].ch;
            if (lowercase && c >= 'A' && c <= 'Z') c += 'a' - 'A';
            return c;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Name entry                                                          */
/* ------------------------------------------------------------------ */

// Fills name_out (needs NAME_LEN + 1 bytes). Returns false if cancelled.
//
// Controls:
//   Letter keys  type that letter (alpha mode)    Alpha  toggle letters/digits
//   0-9          type digit (digit mode)          2nd    toggle UPPER/lower
//   Left/Right   move cursor                      Del    backspace
//   Enter        confirm                          Clear  cancel
bool create_new_game_logic(char name_out[NAME_LEN + 1])
{
    char    name[NAME_LEN];
    uint8_t cursor = 0;
    bool    lowercase  = false;

    for (uint8_t i = 0; i < NAME_LEN; i++) name[i] = ' ';

    while (kb_AnyKey());           // swallow the key press that got us here

    for (;;) {
        // ---- Draw ----
        gfx_FillScreen(BG_COLOR);
        gfx_SetTextFGColor(TEXT_COLOR);
        gfx_SetColor(TEXT_COLOR);

        gfx_SetTextScale(2, 2);
        gfx_SetTextXY(70, 50);
        gfx_PrintString("ENTER NAME");

        for (uint8_t i = 0; i < NAME_LEN; i++) {
            uint16_t x = 80 + i * 32;
            gfx_SetTextXY(x, 100);
            gfx_PrintChar(name[i] == ' ' ? '_' : name[i]);
            if (i == cursor) {
                gfx_HorizLine(x - 2, 120, 16);   // cursor underline
            }
        }

        gfx_SetTextScale(1, 1);
        gfx_SetTextXY(40, 170);
        gfx_PrintString("Alpha: case          2nd: OK");
        gfx_SetTextXY(40, 185);
        gfx_PrintString("Del: backspace  Clear: exit");
        gfx_SwapDraw();

        // ---- Input ----
        kb_Scan();

        if (kb_Data[6] & kb_Clear) {
            while (kb_AnyKey());
            return false;
        }

        if (kb_Data[2] & kb_Alpha) lowercase = !lowercase;

        else if ((kb_Data[7] & kb_Right) && cursor < NAME_LEN - 1) cursor++;
        else if ((kb_Data[7] & kb_Left)  && cursor > 0)            cursor--;

        else if (kb_Data[1] & kb_Del) {
            // Clear the current slot; if it's already blank, step back first
            if (name[cursor] == ' ' && cursor > 0) cursor--;
            name[cursor] = ' ';
        }

        else if (kb_IsDown(kb_Key2nd)) {
            // Strip trailing blanks
            uint8_t len = NAME_LEN;
            while (len > 0 && name[len - 1] == ' ') len--;

            if (len > 0) {             // must contain at least one character
                for (uint8_t i = 0; i < len; i++) name_out[i] = name[i];
                name_out[len] = '\0';
                while (kb_AnyKey());
                return true;
            }
        }

        else {
            char c = scan_char(lowercase);
            if (c) {
                name[cursor] = c;
                if (cursor < NAME_LEN - 1) cursor++;
            }
        }

        while (kb_AnyKey());           // one action per key press
    }
}

/* ------------------------------------------------------------------ */
/* Save select                                                         */
/* ------------------------------------------------------------------ */

// Returns true if the player confirmed deleting the save
static bool confirm_delete(const char *name)
{
    while (kb_AnyKey());

    for (;;) {
        gfx_FillScreen(BG_COLOR);
        gfx_SetTextFGColor(TEXT_COLOR);

        gfx_SetTextScale(2, 2);
        gfx_SetTextXY(60, 60);
        gfx_PrintString("DELETE SAVE?");

        gfx_SetTextScale(1, 1);
        gfx_SetTextXY(60, 110);
        gfx_PrintString(name);

        gfx_SetTextXY(60, 140);
        gfx_PrintString("This cannot be undone.");

        gfx_SetTextXY(60, 180);
        gfx_PrintString("2nd/Enter: delete");
        gfx_SetTextXY(60, 195);
        gfx_PrintString("Clear/Alpha: cancel");
        gfx_SwapDraw();

        kb_Scan();

        if ((kb_Data[1] & kb_2nd) || (kb_Data[6] & kb_Enter)) {
            while (kb_AnyKey());
            return true;
        }
        if ((kb_Data[6] & kb_Clear) || kb_IsDown(kb_KeyAlpha)) {
            while (kb_AnyKey());
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

    for (uint8_t i = 0; i < MAX_SAVES; i++) {
        used[i] = save_read(i, &slots[i]);
    }

    while (kb_AnyKey());

    for (;;) {
        // ---- Draw ----
        gfx_FillScreen(BG_COLOR);
        gfx_SetTextFGColor(TEXT_COLOR);

        gfx_SetTextScale(2, 2);
        gfx_SetTextXY(70, 30);
        gfx_PrintString("SELECT SAVE");

        gfx_SetTextScale(1, 1);
        for (uint8_t i = 0; i < MAX_SAVES; i++) {
            uint8_t y = 80 + i * 30;

            gfx_SetTextXY(60, y);
            gfx_PrintString(i == sel ? ">" : " ");

            gfx_SetTextXY(75, y);
            gfx_PrintUInt(i + 1, 1);
            gfx_PrintString(". ");

            if (used[i]) {
                gfx_PrintString(slots[i].name);
                gfx_PrintString("  ");
                gfx_PrintString(get_area_name(slots[i].area));
            } else {
                gfx_PrintString("- New Game -");
            }
        }

        gfx_SetTextXY(60, 190);
        gfx_PrintString("2nd: select   Alpha: back");
        gfx_SetTextXY(60, 205);
        gfx_PrintString("Clear: delete save");
        gfx_SwapDraw();

        // ---- Input ----
        kb_Scan();

        if (kb_IsDown(kb_KeyAlpha)) {
            while (kb_AnyKey());
            return -1;
        }
        if ((kb_Data[7] & kb_Up)   && sel > 0)             sel--;
        if ((kb_Data[7] & kb_Down) && sel < MAX_SAVES - 1) sel++;

        if (kb_Data[6] & kb_Clear) {
            while (kb_AnyKey());

            if (used[sel] && confirm_delete(slots[sel].name)) {
                if (save_delete(sel)) {
                    used[sel] = false;
                    memset(&slots[sel], 0, sizeof(GameSave));
                }
            }
            continue;
        }

        if (kb_Data[1] & kb_2nd) {
            while (kb_AnyKey());

            if (used[sel]) {
                *out_save = slots[sel];
                return (int8_t)sel;
            }

            // Empty slot: create a new game here
            char name[NAME_LEN + 1];
            if (create_new_game_logic(name)) {
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
            continue;
        }

        while (kb_AnyKey());
    }
}

/* ------------------------------------------------------------------ */
/* Title                                                               */
/* ------------------------------------------------------------------ */

bool title_logic(Renderer *renderer, Camera *camera, Player *player, uint8_t action_key_down, GameState *current_state, TileMap *current_map, AreaId *current_area, Tile *tileset, uint8_t *save_slot, GameSave *save)
{
    // Exit game
    if (kb_IsDown(kb_KeyClear))
        return false;

    // Clear the screen
    gfx_FillScreen(BG_COLOR);
    gfx_SetTextFGColor(TEXT_COLOR);

    // Title
    gfx_SetTextScale(2, 2);
    gfx_SetTextXY(100, 60);
    gfx_PrintString("ELYNDRA");

    // Subtitle
    gfx_SetTextScale(1, 1);
    gfx_SetTextXY(95, 100);
    gfx_PrintString("A TI-84 Adventure");

    // Start prompt
    gfx_SetTextXY(95, 180);
    gfx_PrintString("Press 2nd to Start");

    // Check for start
    if (action_key_down) {
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
            gfx_SwapDraw();

            // Draw first frame
            rendering_draw_initial(renderer, current_map, tileset, camera, player);
        }
    }

    gfx_SwapDraw();

    return true;
}