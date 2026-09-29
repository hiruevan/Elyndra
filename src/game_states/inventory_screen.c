#include <graphx.h>
#include <keypadc.h>
#include <stdio.h>
#include <string.h>

#include "inventory_screen.h"
#include "libs/inventory.h"
#include "libs/player.h"
#include "interactions/items.h"

/* ---- layout constants -------------------------------------------- */
#define INV_TAB_COUNT       CATEGORY_COUNT   /* 4: consumable/key/weapon/armor */
#define INV_ROWS_VISIBLE    7
#define INV_LIST_X          20
#define INV_LIST_Y          55
#define INV_ROW_HEIGHT      22
#define INV_LIST_WIDTH      150
#define INV_PANEL_X         190
#define INV_PANEL_Y         55
#define INV_PANEL_WIDTH     115
#define INV_PANEL_HEIGHT    150
#define INV_TAB_WIDTH       64
#define INV_TAB_GAP         4

/* Palette indices */
#define COL_BG        1
#define COL_PANEL_BG  2
#define COL_BORDER    223
#define COL_TEXT      223
#define COL_TEXT_DIM  191
#define COL_TEXT_STAT 200
#define COL_SELECTED  250
#define COL_TAB_ON    250
#define COL_TAB_OFF   2

static const char *inv_tab_labels[INV_TAB_COUNT] = {
    "ITEMS",    /* CATEGORY_CONSUMABLE */
    "KEY",      /* CATEGORY_KEY        */
    "WEAPONS",  /* CATEGORY_WEAPON     */
    "ARMOR"     /* CATEGORY_ARMOR      */
};

/* ---- local menu state (persists across frames, like pause_screen) */
static uint8_t inv_tab    = 0;   /* active ItemCategory            */
static uint8_t inv_cursor = 0;   /* selected row within filtered list */
static uint8_t inv_scroll = 0;   /* first visible row (scrolling)  */

static uint8_t inv_prev_up     = 0;
static uint8_t inv_prev_down   = 0;
static uint8_t inv_prev_left   = 0;
static uint8_t inv_prev_right  = 0;
static uint8_t inv_prev_action = 1;
static uint8_t inv_prev_back  = 0;

/* Slot indices (into inv->slots[]) belonging to the active tab,
 * rebuilt once per frame. */
static uint8_t inv_filtered[INVENTORY_SIZE];
static uint8_t inv_filtered_count = 0;

static uint8_t inv_key_just_pressed(uint8_t was_down, uint8_t is_down)
{
    return is_down && !was_down;
}

/* Scan inv->slots[] for non-empty slots whose item's category
 * matches the active tab; fill inv_filtered[] with their indices. */
static void inv_rebuild_filter(const Inventory *inv)
{
    inv_filtered_count = 0;

    for (uint8_t i = 0; i < INVENTORY_SIZE; i++)
    {
        const InventorySlot *slot = &inv->slots[i];
        if (slot->id == ITEM_NONE)
            continue;

        if (item_defs[slot->id].category == (ItemCategory)inv_tab)
            inv_filtered[inv_filtered_count++] = i;
    }
}

/* ---- drawing -------------------------------------------------------- */

static void draw_box(int x, int y, int w, int h)
{
    gfx_SetColor(COL_PANEL_BG);
    gfx_FillRectangle(x, y, w, h);
    gfx_SetColor(COL_BORDER);
    gfx_Rectangle(x, y, w, h);
    gfx_Rectangle(x + 2, y + 2, w - 4, h - 4); /* double-frame BoF look */
}

static void draw_tabs(void)
{
    int x = INV_LIST_X;
    for (uint8_t i = 0; i < INV_TAB_COUNT; i++)
    {
        gfx_SetColor(i == inv_tab ? COL_TAB_ON : COL_TAB_OFF);
        gfx_FillRectangle(x, 15, INV_TAB_WIDTH, 26);
        gfx_SetColor(COL_BORDER);
        gfx_Rectangle(x, 15, INV_TAB_WIDTH, 26);

        gfx_SetTextFGColor(i == inv_tab ? COL_BG : COL_TEXT);
        gfx_SetTextScale(1, 1);
        gfx_PrintStringXY(inv_tab_labels[i], x + 4, 24);

        x += INV_TAB_WIDTH + INV_TAB_GAP;
    }
}

static void draw_item_list(const Inventory *inv)
{
    draw_box(INV_LIST_X, INV_LIST_Y, INV_LIST_WIDTH, INV_ROWS_VISIBLE * INV_ROW_HEIGHT + 10);

    if (inv_filtered_count == 0)
    {
        gfx_SetTextFGColor(COL_TEXT_DIM);
        gfx_PrintStringXY("(empty)", INV_LIST_X + 14, INV_LIST_Y + 14);
        return;
    }

    for (uint8_t row = 0; row < INV_ROWS_VISIBLE; row++)
    {
        uint8_t list_idx = inv_scroll + row;
        if (list_idx >= inv_filtered_count)
            break;

        uint8_t slot_idx = inv_filtered[list_idx];
        const InventorySlot *slot = &inv->slots[slot_idx];
        const ItemDef *def = &item_defs[slot->id];

        int y = INV_LIST_Y + 5 + row * INV_ROW_HEIGHT;

        if (list_idx == inv_cursor)
        {
            gfx_SetColor(COL_SELECTED);
            gfx_FillRectangle(INV_LIST_X + 4, y, INV_LIST_WIDTH - 8, INV_ROW_HEIGHT - 2);
            gfx_SetTextFGColor(COL_BG);
            gfx_PrintStringXY(">", INV_LIST_X + 8, y + 6);
        }
        else
        {
            gfx_SetTextFGColor(COL_TEXT);
        }

        gfx_PrintStringXY(def->name, INV_LIST_X + 22, y + 6);

        if (def->stackable && slot->count > 1)
        {
            char qty_buf[8];
            sprintf(qty_buf, "x%d", slot->count);
            gfx_PrintStringXY(qty_buf, INV_LIST_X + INV_LIST_WIDTH - 34, y + 6);
        }
    }

    if (inv_filtered_count > INV_ROWS_VISIBLE)
    {
        int track_h = INV_ROWS_VISIBLE * INV_ROW_HEIGHT;
        int thumb_h = track_h * INV_ROWS_VISIBLE / inv_filtered_count;
        int thumb_y = INV_LIST_Y + 5 +
            (track_h - thumb_h) * inv_scroll / (inv_filtered_count - INV_ROWS_VISIBLE);

        gfx_SetColor(COL_BORDER);
        gfx_FillRectangle(INV_LIST_X + INV_LIST_WIDTH - 6, thumb_y, 3, thumb_h);
    }
}

/* naive word-wrap into the detail panel; returns the y position
 * just below the wrapped text so a stat line can follow it */
static int draw_wrapped_text(const char *src, int x, int y, uint8_t max_w)
{
    char line[64];

    while (src && *src)
    {
        uint8_t len = 0;
        uint8_t last_space = 0;

        while (src[len])
        {
            if (src[len] == ' ')
                last_space = len;

            memcpy(line, src, len + 1);
            line[len + 1] = '\0';

            if (gfx_GetStringWidth(line) > max_w)
            {
                if (last_space > 0)
                    len = last_space;      /* break at last word boundary */
                break;                      /* else hard-break mid-word   */
            }
            len++;
        }

        if (len == 0)
            len = 1; /* guard against an infinite loop on one huge glyph */

        memcpy(line, src, len);
        line[len] = '\0';
        gfx_PrintStringXY(line, x, y);

        src += len;
        while (*src == ' ')
            src++;
        y += 12;

        if (y > INV_PANEL_Y + INV_PANEL_HEIGHT - 28)
            break; /* out of room */
    }

    return y;
}

static void draw_detail_panel(const Inventory *inv)
{
    draw_box(INV_PANEL_X, INV_PANEL_Y, INV_PANEL_WIDTH, INV_PANEL_HEIGHT);

    if (inv_filtered_count == 0)
        return;

    uint8_t slot_idx = inv_filtered[inv_cursor];
    const InventorySlot *slot = &inv->slots[slot_idx];
    const ItemDef *def = &item_defs[slot->id];

    gfx_SetTextFGColor(COL_SELECTED);
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY(def->name, INV_PANEL_X + 10, INV_PANEL_Y + 12);

    gfx_SetTextFGColor(COL_TEXT);
    int y = draw_wrapped_text(def->description, INV_PANEL_X + 10, INV_PANEL_Y + 34, INV_PANEL_WIDTH - 20);

    /* stat line pulled from the ItemDef union, per category */
    char stat_buf[24];
    switch (def->category)
    {
        case CATEGORY_CONSUMABLE:
            sprintf(stat_buf, "Heals %d HP", def->stats.consumable.heal_amount);
            break;
        case CATEGORY_WEAPON:
            sprintf(stat_buf, "ATK +%d", def->stats.weapon.attack);
            break;
        case CATEGORY_ARMOR:
            sprintf(stat_buf, "DEF +%d", def->stats.armor.defense);
            break;
        default:
            stat_buf[0] = '\0';
            break;
    }

    if (stat_buf[0])
    {
        gfx_SetTextFGColor(COL_TEXT_STAT);
        gfx_PrintStringXY(stat_buf, INV_PANEL_X + 10, y + 6);
    }
}

void draw_gold(Inventory *inv)
{
    char g_buff[16];
    sprintf(g_buff, "Gold: %d", inv->gold);

    gfx_SetTextFGColor(COL_TEXT);
    gfx_PrintStringXY(g_buff, INV_PANEL_X + 10, INV_PANEL_Y + INV_PANEL_HEIGHT + 5);
}

void inventory_screen_open(void)
{
    inv_prev_up     = kb_IsDown(kb_KeyUp);
    inv_prev_down   = kb_IsDown(kb_KeyDown);
    inv_prev_left   = kb_IsDown(kb_KeyLeft);
    inv_prev_right  = kb_IsDown(kb_KeyRight);
    inv_prev_action = kb_IsDown(kb_Key2nd);
    inv_prev_back  = kb_IsDown(kb_KeyAlpha);

    /* also reset menu position so re-opening starts clean */
    inv_tab    = 0;
    inv_cursor = 0;
    inv_scroll = 0;
}

bool inventory_screen(Player *player)
{
    Inventory *inv = &player->inventory;

    inv_rebuild_filter(inv);
    if (inv_cursor >= inv_filtered_count && inv_filtered_count > 0)
        inv_cursor = inv_filtered_count - 1;

    gfx_FillScreen(COL_BG);

    draw_tabs();
    draw_item_list(inv);
    draw_detail_panel(inv);
    draw_gold(inv);

    /* ---- input ---- */
    uint8_t up     = kb_IsDown(kb_KeyUp);
    uint8_t down   = kb_IsDown(kb_KeyDown);
    uint8_t left   = kb_IsDown(kb_KeyLeft);
    uint8_t right  = kb_IsDown(kb_KeyRight);
    uint8_t action = kb_IsDown(kb_Key2nd);
    uint8_t back  = kb_IsDown(kb_KeyAlpha);

    if (inv_key_just_pressed(inv_prev_left, left) && inv_tab > 0)
    {
        inv_tab--;
        inv_cursor = 0;
        inv_scroll = 0;
    }

    if (inv_key_just_pressed(inv_prev_right, right) && inv_tab < INV_TAB_COUNT - 1)
    {
        inv_tab++;
        inv_cursor = 0;
        inv_scroll = 0;
    }

    if (inv_key_just_pressed(inv_prev_up, up) && inv_filtered_count > 0)
    {
        if (inv_cursor > 0)
            inv_cursor--;
        if (inv_cursor < inv_scroll)
            inv_scroll = inv_cursor;
    }

    if (inv_key_just_pressed(inv_prev_down, down) && inv_filtered_count > 0)
    {
        if (inv_cursor < inv_filtered_count - 1)
            inv_cursor++;
        if (inv_cursor >= inv_scroll + INV_ROWS_VISIBLE)
            inv_scroll = inv_cursor - INV_ROWS_VISIBLE + 1;
    }

    if (inv_key_just_pressed(inv_prev_action, action) && inv_filtered_count > 0)
    {
        uint8_t slot_idx = inv_filtered[inv_cursor];
        InventorySlot *slot = &inv->slots[slot_idx];
        const ItemDef *def = &item_defs[slot->id];

        if (def->category == CATEGORY_CONSUMABLE)
        {
            /* TODO: apply def->stats.consumable.heal_amount to the ---------------------------------------------------
             * player, then consume one: */
            inventory_remove(inv, slot->id, 1);
            inv_rebuild_filter(inv);
            if (inv_cursor >= inv_filtered_count && inv_cursor > 0)
                inv_cursor--;
        }
        else if (def->category == CATEGORY_WEAPON || def->category == CATEGORY_ARMOR)
        {
            // TODO: hook up equip logic here -------------------------------------------------------
        }
    }

    inv_prev_up     = up;
    inv_prev_down   = down;
    inv_prev_left   = left;
    inv_prev_right  = right;
    inv_prev_action = action;

    if (inv_key_just_pressed(inv_prev_back, back))
    {
        inv_prev_back = back;
        gfx_SwapDraw();
        return false; /* back to pause menu */
    }
    inv_prev_back = back;

    gfx_SwapDraw();
    return true;
}