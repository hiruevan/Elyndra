#include <graphx.h>
#include <keypadc.h>
#include <stdio.h>
#include <string.h>

#include "inventory_screen.h"
#include "game_state.h"
#include "../interactions/items.h"
#include "../libs/inventory.h"
#include "../libs/player.h"
#include "../interactions/items.h"
#include "../libs/input.h"
#include "../libs/area.h"
#include "../libs/rendering.h"

/* ---- tabs ------------------------------------------------------------ */
/* Tab 0 is PARTY; tabs 1..CATEGORY_COUNT map to ItemCategory (tab - 1). */
#define INV_TAB_PARTY       0
#define INV_TAB_ITEM_BASE   1
#define INV_TAB_COUNT       (CATEGORY_COUNT + 1)

#define NO_GRAB 0xFF

/* ---- layout constants -------------------------------------------------- */
#define INV_ROWS_VISIBLE    7
#define INV_ROW_HEIGHT      22
#define ST_ROWS_VISIBLE     4
#define ST_ROW_HEIGHT       36

#define LIST_X              20
#define LIST_Y              55
#define LIST_WIDTH          150
#define PANEL_X             190
#define PANEL_Y             55
#define PANEL_WIDTH         115
#define PANEL_HEIGHT        150

#define TAB_WIDTH           54
#define TAB_GAP             3
#define HEADER_X            20
#define HEADER_Y            15
#define HEADER_W            285
#define HEADER_H            26

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
#define COL_BAR_BG    0
#define COL_BAR_HP    25
#define COL_BAR_MP    199
#define COL_BAR_EXP   229

static const char *inv_tab_labels[INV_TAB_COUNT] = {
    "PARTY",    /* INV_TAB_PARTY       */
    "ITEM",     /* CATEGORY_CONSUMABLE */
    "KEY",      /* CATEGORY_KEY        */
    "WEAPON",   /* CATEGORY_WEAPON     */
    "ARMOR"     /* CATEGORY_ARMOR      */
};

/* ---- local menu state (persists across frames) ------------------------- */
static uint8_t inv_tab    = 0;
static uint8_t inv_cursor = 0;   /* item list cursor (filtered index) */
static uint8_t inv_scroll = 0;
static uint8_t pty_cursor = 0;   /* party tab cursor                  */
static uint8_t pty_scroll = 0;

static uint8_t prev_unequip = 0;

static bool    inv_picking   = false;  /* choosing a target for an item */
static uint8_t inv_pick_slot = 0;
static uint8_t pk_cursor     = 0;
static uint8_t pk_scroll     = 0;

static bool    pty_grabbing  = false;
static uint8_t pty_grab_from = 0;

static uint8_t inv_filtered[INVENTORY_SIZE];
static uint8_t inv_filtered_count = 0;

static void inv_rebuild_filter(const Inventory *inv)
{
    inv_filtered_count = 0;

    for (uint8_t i = 0; i < INVENTORY_SIZE; i++)
    {
        const InventorySlot *slot = &inv->slots[i];
        if (slot->id == ITEM_NONE)
            continue;

        if (item_defs[slot->id].category == (ItemCategory)(inv_tab - INV_TAB_ITEM_BASE))
            inv_filtered[inv_filtered_count++] = i;
    }
}

/* ---- Party Swapping ------------------------------------------------------ */
static void party_swap(Player *player, uint8_t a, uint8_t b)
{
    if (a >= player->party_count || b >= player->party_count || a == b)
        return;

    PartyMember tmp = player->party[a];
    player->party[a] = player->party[b];
    player->party[b] = tmp;
}

/* ---- input helpers ------------------------------------------------------- */

static bool edge(uint8_t *prev, uint8_t now)
{
    bool pressed = key_just_pressed(*prev, now);
    *prev = now;
    return pressed;
}

static void list_nav(uint8_t count, uint8_t rows, uint8_t *cursor, uint8_t *scroll,
                     bool up, bool down)
{
    if (count == 0)
    {
        *cursor = *scroll = 0;
        return;
    }

    if (*cursor >= count)
        *cursor = count - 1;

    if (up && *cursor > 0)
        (*cursor)--;
    if (down && *cursor < count - 1)
        (*cursor)++;

    if (*cursor < *scroll)
        *scroll = *cursor;
    if (*cursor >= *scroll + rows)
        *scroll = *cursor - rows + 1;
}

/* ---- drawing ------------------------------------------------------------- */

static void draw_box(int x, int y, int w, int h)
{
    gfx_SetColor(COL_PANEL_BG);
    gfx_FillRectangle(x, y, w, h);
    gfx_SetColor(COL_BORDER);
    gfx_Rectangle(x, y, w, h);
    gfx_Rectangle(x + 2, y + 2, w - 4, h - 4);
}

static void draw_tabs(void)
{
    int x = HEADER_X;
    for (uint8_t i = 0; i < INV_TAB_COUNT; i++)
    {
        gfx_SetColor(i == inv_tab ? COL_TAB_ON : COL_TAB_OFF);
        gfx_FillRectangle(x, HEADER_Y, TAB_WIDTH, HEADER_H);
        gfx_SetColor(COL_BORDER);
        gfx_Rectangle(x, HEADER_Y, TAB_WIDTH, HEADER_H);

        gfx_SetTextFGColor(i == inv_tab ? COL_BG : COL_TEXT);
        gfx_SetTextScale(1, 1);
        gfx_PrintStringXY(inv_tab_labels[i], x + 4, HEADER_Y + 9);

        x += TAB_WIDTH + TAB_GAP;
    }
}

static void draw_header(const char *text)
{
    gfx_SetColor(COL_SELECTED);
    gfx_FillRectangle(HEADER_X, HEADER_Y, HEADER_W, HEADER_H);
    gfx_SetColor(COL_BORDER);
    gfx_Rectangle(HEADER_X, HEADER_Y, HEADER_W, HEADER_H);

    gfx_SetTextFGColor(COL_BG);
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY(text, HEADER_X + 8, HEADER_Y + 9);
}

static void draw_bar(int x, int y, int w, int h, uint32_t cur, uint32_t max, uint8_t col)
{
    gfx_SetColor(COL_BAR_BG);
    gfx_FillRectangle(x, y, w, h);

    if (max > 0 && cur > 0)
    {
        int inner = w - 2;
        int fill = (int)((uint32_t)inner * cur / max);
        if (fill < 1)
            fill = 1;
        if (fill > inner)
            fill = inner;
        gfx_SetColor(col);
        gfx_FillRectangle(x + 1, y + 1, fill, h - 2);
    }

    gfx_SetColor(COL_BORDER);
    gfx_Rectangle(x, y, w, h);
}

static void draw_scrollbar(uint8_t count, uint8_t rows, int row_h, uint8_t scroll)
{
    if (count <= rows)
        return;

    int track_h = rows * row_h;
    int thumb_h = track_h * rows / count;
    int thumb_y = LIST_Y + 5 + (track_h - thumb_h) * scroll / (count - rows);

    gfx_SetColor(COL_BORDER);
    gfx_FillRectangle(LIST_X + LIST_WIDTH - 6, thumb_y, 3, thumb_h);
}

/* One-line key hint for the current tab, drawn under the list. */
static void draw_helper(bool grabbing, bool using)
{
    const char *hint = NULL;

    if (inv_tab == INV_TAB_PARTY)
    {
        hint = grabbing ? "2nd: place    Alpha: cancel"
                        : "2nd: switch    Clear: unequip";
    }
    else
    {
        switch ((ItemCategory)(inv_tab - INV_TAB_ITEM_BASE))
        {
            case CATEGORY_CONSUMABLE: hint = "2nd: use";   break;
            case CATEGORY_WEAPON:
            case CATEGORY_ARMOR:      hint = "2nd: equip"; break;
            default:                  break;   /* KEY tab: no action */
        }
    }
    if (using)
        hint = "2nd: select    Alpha: cancel";

    if (hint)
    {
        gfx_SetTextScale(1, 1);
        gfx_SetTextFGColor(COL_TEXT_DIM);
        gfx_PrintStringXY(hint, LIST_X, LIST_Y + INV_ROWS_VISIBLE * INV_ROW_HEIGHT + 17);
    }
}

static void draw_item_list(const Inventory *inv)
{
    draw_box(LIST_X, LIST_Y, LIST_WIDTH, INV_ROWS_VISIBLE * INV_ROW_HEIGHT + 10);

    if (inv_filtered_count == 0)
    {
        gfx_SetTextFGColor(COL_TEXT_DIM);
        gfx_PrintStringXY("(empty)", LIST_X + 14, LIST_Y + 14);
        return;
    }

    for (uint8_t row = 0; row < INV_ROWS_VISIBLE; row++)
    {
        uint8_t list_idx = inv_scroll + row;
        if (list_idx >= inv_filtered_count)
            break;

        const InventorySlot *slot = &inv->slots[inv_filtered[list_idx]];
        const ItemDef *def = &item_defs[slot->id];
        int y = LIST_Y + 5 + row * INV_ROW_HEIGHT;

        if (list_idx == inv_cursor)
        {
            gfx_SetColor(COL_SELECTED);
            gfx_FillRectangle(LIST_X + 4, y, LIST_WIDTH - 8, INV_ROW_HEIGHT - 2);
            gfx_SetTextFGColor(COL_BG);
            gfx_PrintStringXY(">", LIST_X + 8, y + 6);
        }
        else
        {
            gfx_SetTextFGColor(COL_TEXT);
        }

        gfx_PrintStringXY(def->name, LIST_X + 22, y + 6);

        if (def->stackable && slot->count > 1)
        {
            char qty_buf[8];
            sprintf(qty_buf, "x%d", slot->count);
            gfx_PrintStringXY(qty_buf, LIST_X + LIST_WIDTH - 34, y + 6);
        }
    }

    draw_scrollbar(inv_filtered_count, INV_ROWS_VISIBLE, INV_ROW_HEIGHT, inv_scroll);
}

/* pick_def == NULL -> plain overview; otherwise dim unusable targets. */
static void draw_party_list(const Player *player, uint8_t cursor, uint8_t scroll,
                            const ItemDef *pick_def, uint8_t grab)
{
    draw_box(LIST_X, LIST_Y, LIST_WIDTH, INV_ROWS_VISIBLE * INV_ROW_HEIGHT + 10);

    if (player->party_count == 0)
    {
        gfx_SetTextFGColor(COL_TEXT_DIM);
        gfx_PrintStringXY("(no party)", LIST_X + 14, LIST_Y + 14);
        return;
    }

    for (uint8_t row = 0; row < ST_ROWS_VISIBLE; row++)
    {
        uint8_t idx = scroll + row;
        if (idx >= player->party_count)
            break;

        const PartyMember *m = &player->party[idx];
        int y = LIST_Y + 5 + row * ST_ROW_HEIGHT;
        int x = LIST_X + 4;
        uint8_t fg = COL_TEXT;
        char buf[16];

        if (idx == grab)
        {
            gfx_SetColor(COL_TEXT_STAT);              /* distinct fill */
            gfx_FillRectangle(x, y, LIST_WIDTH - 8, ST_ROW_HEIGHT - 2);
            fg = COL_BG;
        }
        else if (idx == cursor)
        {
            gfx_SetColor(COL_SELECTED);
            gfx_FillRectangle(x, y, LIST_WIDTH - 8, ST_ROW_HEIGHT - 2);
            fg = COL_BG;
        }
        else if (pick_def && !member_can_target(m, pick_def))
        {
            fg = COL_TEXT_DIM;
        }

        gfx_SetTextFGColor(fg);

        gfx_PrintStringXY(m->name, x + 6, y + 4);
        sprintf(buf, "Lv%d", m->level);
        gfx_PrintStringXY(buf, x + LIST_WIDTH - 8 - 40, y + 4);

        gfx_PrintStringXY("HP", x + 6, y + 14);
        draw_bar(x + 24, y + 14, 52, 8, m->hp, m->max_hp, COL_BAR_HP);
        sprintf(buf, "%d/%d", m->hp, m->max_hp);
        gfx_SetTextFGColor(fg);
        gfx_PrintStringXY(buf, x + 80, y + 14);

        gfx_PrintStringXY("MP", x + 6, y + 25);
        draw_bar(x + 24, y + 25, 52, 8, m->mp, m->max_mp, COL_BAR_MP);
        sprintf(buf, "%d/%d", m->mp, m->max_mp);
        gfx_SetTextFGColor(fg);
        gfx_PrintStringXY(buf, x + 80, y + 25);
    }

    draw_scrollbar(player->party_count, ST_ROWS_VISIBLE, ST_ROW_HEIGHT, scroll);
}

/* naive word-wrap into the detail panel; returns the y just below the text */
static int draw_wrapped_text(const char *src, int x, int y, uint8_t max_w)
{
    char line[64];

    while (src && *src)
    {
        uint8_t len = 0;
        uint8_t last_space = 0;

        while (src[len] && len < sizeof(line) - 2)
        {
            if (src[len] == ' ')
                last_space = len;

            memcpy(line, src, len + 1);
            line[len + 1] = '\0';

            if (gfx_GetStringWidth(line) > max_w)
            {
                if (last_space > 0)
                    len = last_space;
                break;
            }
            len++;
        }

        if (len == 0)
            len = 1;

        memcpy(line, src, len);
        line[len] = '\0';
        gfx_PrintStringXY(line, x, y);

        src += len;
        while (*src == ' ')
            src++;
        y += 12;

        if (y > PANEL_Y + PANEL_HEIGHT - 28)
            break;
    }

    return y;
}

static void draw_item_detail(const Inventory *inv)
{
    draw_box(PANEL_X, PANEL_Y, PANEL_WIDTH, PANEL_HEIGHT);

    if (inv_filtered_count == 0)
        return;

    const InventorySlot *slot = &inv->slots[inv_filtered[inv_cursor]];
    const ItemDef *def = &item_defs[slot->id];

    gfx_SetTextFGColor(COL_SELECTED);
    gfx_SetTextScale(1, 1);
    gfx_PrintStringXY(def->name, PANEL_X + 10, PANEL_Y + 12);

    gfx_SetTextFGColor(COL_TEXT);
    int y = draw_wrapped_text(def->description, PANEL_X + 10, PANEL_Y + 34, PANEL_WIDTH - 20);

    char stat_buf[24];
    switch (def->category)
    {
        case CATEGORY_CONSUMABLE:
            if (def->stats.consumable.heal_amount > 0)
                sprintf(stat_buf, "Heals %d HP", def->stats.consumable.heal_amount);
            else
                sprintf(stat_buf, "Recovers %d MP", def->stats.consumable.mp_regain);
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
        gfx_PrintStringXY(stat_buf, PANEL_X + 10, y + 6);
    }
}

static void draw_equip_line(const char *label, ItemId id, int x, int y, uint8_t max_w)
{
    char buf[24];
    const char *name = (id == ITEM_NONE) ? "-" : item_defs[id].name;

    sprintf(buf, "%s %s", label, name);
    while (strlen(buf) > 3 && gfx_GetStringWidth(buf) > max_w)
        buf[strlen(buf) - 1] = '\0';

    gfx_PrintStringXY(buf, x, y);
}

static void draw_member_detail(const Player *player, uint8_t idx, const ItemDef *pick_def)
{
    draw_box(PANEL_X, PANEL_Y, PANEL_WIDTH, PANEL_HEIGHT);

    if (player->party_count == 0)
        return;

    const PartyMember *m = &player->party[idx];
    int x = PANEL_X + 10;
    int y = PANEL_Y + 12;
    char buf[24];

    gfx_SetTextScale(1, 1);
    gfx_SetTextFGColor(COL_SELECTED);
    if (idx == 0)
        sprintf(buf, "%s (Leader)", m->name);
    else if (idx < 3)
        sprintf(buf, "%s (Battle)", m->name);
    else 
        sprintf(buf, "%s", m->name);
    gfx_PrintStringXY(buf, x, y);

    gfx_SetTextFGColor(COL_TEXT);
    sprintf(buf, "Level %d", m->level);
    gfx_PrintStringXY(buf, x, y + 16);

    sprintf(buf, "EXP %lu", (unsigned long)m->exp);
    gfx_PrintStringXY(buf, x, y + 32);

    uint32_t to_next = (m->exp_next > m->exp) ? (m->exp_next - m->exp) : 0;
    sprintf(buf, "Next %lu", (unsigned long)to_next);
    gfx_PrintStringXY(buf, x, y + 44);
    draw_bar(x, y + 56, PANEL_WIDTH - 20, 8, m->exp, m->exp_next, COL_BAR_EXP);

    gfx_SetTextFGColor(COL_TEXT_STAT);

    sprintf(buf, "ATK %d", member_attack(m));
    gfx_PrintStringXY(buf, x, y + 72);
    sprintf(buf, "DEF %d", member_defense(m));
    gfx_PrintStringXY(buf, x, y + 84);

    sprintf(buf, "SPD %d", member_speed(m));
    gfx_PrintStringXY(buf, x, y + 96);

    if (!pick_def)
    {
        gfx_SetTextFGColor(COL_TEXT);
        draw_equip_line("W:", m->weapon, x, y + 108, PANEL_WIDTH - 20);
        draw_equip_line("A:", m->armor,  x, y + 120, PANEL_WIDTH - 20);
    }

    /* item preview while picking: Stats before -> after */
    if (pick_def)
    {
        bool ok = member_can_target(m, pick_def);
        gfx_SetTextFGColor(ok ? COL_SELECTED : COL_TEXT_DIM);

        if (!ok)
        {
            sprintf(buf, pick_def->category == CATEGORY_CONSUMABLE ? "No effect"
                                                                : "Can't equip");
            gfx_PrintStringXY(buf, x, y + 122);
        }
        else if (pick_def->category == CATEGORY_WEAPON ||
             pick_def->category == CATEGORY_ARMOR)
        {
            ItemId new_id = item_id_of(pick_def);
            bool is_wpn = (pick_def->category == CATEGORY_WEAPON);

            int atk_or_def_now, atk_or_def_new, spd_new;
            const char *label;

            if (is_wpn)
            {
                label = "ATK";
                atk_or_def_now = member_attack(m);
                atk_or_def_new = m->attack + pick_def->stats.weapon.attack;
                spd_new = speed_with(m, new_id, m->armor);
            }
            else
            {
                label = "DEF";
                atk_or_def_now = member_defense(m);
                atk_or_def_new = m->defense + pick_def->stats.armor.defense;
                spd_new = speed_with(m, m->weapon, new_id);
            }

            sprintf(buf, "%s %d > %d", label, atk_or_def_now, atk_or_def_new);
            gfx_PrintStringXY(buf, x, y + 112);

            sprintf(buf, "SPD %d > %d", member_speed(m), spd_new);
            gfx_PrintStringXY(buf, x, y + 124);
        }
        else if (pick_def->stats.consumable.heal_amount > 0)
        {
            uint16_t room = m->max_hp - m->hp;
            uint16_t heal = pick_def->stats.consumable.heal_amount;
            sprintf(buf, "HP %d > %d", m->hp, m->hp + ((heal > room) ? room : heal));
        }
        else
        {
            uint16_t room = m->max_mp - m->mp;
            uint16_t rec = pick_def->stats.consumable.mp_regain;
            sprintf(buf, "MP %d > %d", m->mp, m->mp + ((rec > room) ? room : rec));
        }
    }
}

static void draw_gold(const Inventory *inv)
{
    char g_buff[16];
    sprintf(g_buff, "Gold: %d", inv->gold);

    gfx_SetTextFGColor(COL_TEXT);
    gfx_PrintStringXY(g_buff, PANEL_X + 10, PANEL_Y + PANEL_HEIGHT + 5);
}

/* ---- the screen ---------------------------------------------------------- */

bool inventory_screen(Player *player, uint8_t
    *inv_prev_up, uint8_t *inv_prev_down, uint8_t *inv_prev_left, uint8_t *inv_prev_right, uint8_t *inv_prev_action, uint8_t *inv_prev_back, uint8_t *inv_prev_inv,
    GameState *current_state, TileMap *current_map, Tile *tileset, Renderer *renderer, Camera *camera)
{
    Inventory *inv = &player->inventory;

    /* ---- input: edges computed once for every mode ---- */
    bool up_p     = edge(inv_prev_up,     kb_IsDown(kb_KeyUp));
    bool down_p   = edge(inv_prev_down,   kb_IsDown(kb_KeyDown));
    bool left_p   = edge(inv_prev_left,   kb_IsDown(kb_KeyLeft));
    bool right_p  = edge(inv_prev_right,  kb_IsDown(kb_KeyRight));
    bool action_p = edge(inv_prev_action, kb_IsDown(kb_Key2nd));
    bool back_p   = edge(inv_prev_back,   kb_IsDown(kb_KeyAlpha));
    bool inv_p    = edge(inv_prev_inv,    kb_IsDown(kb_KeyDel));

    /* ---- target picker (using a consumable) ---- */
    if (inv_picking)
    {
        InventorySlot *pslot = &inv->slots[inv_pick_slot];
        const ItemDef *pdef = &item_defs[pslot->id];

        list_nav(player->party_count, ST_ROWS_VISIBLE, &pk_cursor, &pk_scroll, up_p, down_p);

        if (action_p && player->party_count > 0 &&
            member_can_target(&player->party[pk_cursor], pdef))
        {
            ItemId id = pslot->id;              /* capture before the slot changes */
            PartyMember *m = &player->party[pk_cursor];

            if (pdef->category == CATEGORY_CONSUMABLE)
            {
                if (status_apply_item(m, pdef))
                    inventory_remove(inv, id, 1);
            }
            else
            {
                equip_item(inv, m, id);
            }
            inv_picking = false;
        }
        else if (back_p || inv_p)
        {
            inv_picking = false;
        }

        gfx_FillScreen(COL_BG);
        char title[40];
        sprintf(title, "%s %s on whom?",
            pdef->category == CATEGORY_CONSUMABLE ? "Use" : "Equip", pdef->name);
        draw_header(title);
        draw_party_list(player, pk_cursor, pk_scroll, pdef, NO_GRAB);
        draw_member_detail(player, pk_cursor, pdef);
        draw_gold(inv);
        draw_helper(false, true);
        gfx_SwapDraw();
        return true;
    }

    /* ---- tab switching ---- */
    if (left_p && inv_tab > 0 && !pty_grabbing)
    {
        inv_tab--;
        inv_cursor = inv_scroll = 0;
    }
    if (right_p && inv_tab < INV_TAB_COUNT - 1 && !pty_grabbing)
    {
        inv_tab++;
        inv_cursor = inv_scroll = 0;
    }

    bool on_party = (inv_tab == INV_TAB_PARTY);

    /* ---- per-tab navigation / actions ---- */
    if (on_party)
    {
        list_nav(player->party_count, ST_ROWS_VISIBLE, &pty_cursor, &pty_scroll, up_p, down_p);

        bool un_p = edge(&prev_unequip, kb_IsDown(kb_KeyClear));

        if (!pty_grabbing && player->party_count > 0)
        {
            PartyMember *m = &player->party[pty_cursor];

            if (un_p)
            {
                if (m->armor != ITEM_NONE)
                    unequip_item(inv, &m->armor);
                if (m->weapon != ITEM_NONE)
                    unequip_item(inv, &m->weapon);
            }
        }

        if (action_p && player->party_count > 1)
        {
            if (!pty_grabbing)
            {
                pty_grabbing  = true;
                pty_grab_from = pty_cursor;
            }
            else
            {
                party_swap(player, pty_grab_from, pty_cursor);  /* no-op if same slot */
                pty_grabbing = false;
                /* cursor stays put, which is now where the grabbed member landed */
            }
        }
    }
    else
    {
        inv_rebuild_filter(inv);
        list_nav(inv_filtered_count, INV_ROWS_VISIBLE, &inv_cursor, &inv_scroll, up_p, down_p);

        if (action_p && inv_filtered_count > 0)
        {
            uint8_t slot_idx = inv_filtered[inv_cursor];
            const ItemDef *def = &item_defs[inv->slots[slot_idx].id];

            if (def->category == CATEGORY_CONSUMABLE ||
                def->category == CATEGORY_WEAPON ||
                def->category == CATEGORY_ARMOR)
            {
                inv_picking = true;
                inv_pick_slot = slot_idx;
                pk_cursor = pk_scroll = 0;
            }
        }
    }

    /* ---- draw ---- */
    gfx_FillScreen(COL_BG);
    draw_tabs();
    if (on_party)
    {
        draw_party_list(player, pty_cursor, pty_scroll, NULL, 
            pty_grabbing ? pty_grab_from : NO_GRAB);
        draw_member_detail(player, pty_cursor, NULL);
    }
    else
    {
        draw_item_list(inv);
        draw_item_detail(inv);
    }
    draw_gold(inv);
    draw_helper(pty_grabbing, false);

    if (back_p && pty_grabbing)
    {
        pty_grabbing = false;
        back_p = false;           /* consume it */
    } else if (back_p) // exit
    {
        gfx_SwapDraw();
        return false; /* back to pause menu */
    }

    if (inv_p)
    {
        pty_grabbing = false;
        *current_state = GAME_EXPLORATION;
        rendering_draw_initial(renderer, current_map, tileset, camera, player);
    }

    gfx_SwapDraw();
    return true;
}