#include <graphx.h>
#include <keypadc.h>
#include <stdio.h>
#include <string.h>

#include "shop_data.h"
#include "../game_states/game_state.h"
#include "../libs/inventory.h"
#include "../libs/player.h"
#include "../libs/input.h"
#include "../libs/area.h"
#include "../libs/rendering.h"
#include "items.h"

/* ---- layout / palette: same values as inventory_screen.c ---------------- */
#define ROWS_VISIBLE   7
#define ROW_HEIGHT     22
#define LIST_X         20
#define LIST_Y         55
#define LIST_WIDTH     150
#define PANEL_X        190
#define PANEL_Y        55
#define PANEL_WIDTH    115
#define PANEL_HEIGHT   150
#define HEADER_X       20
#define HEADER_Y       15
#define TAB_WIDTH      54
#define TAB_GAP        3
#define HEADER_H       26

#define COL_BG         1
#define COL_PANEL_BG   2
#define COL_BORDER     223
#define COL_TEXT       223
#define COL_TEXT_DIM   191
#define COL_TEXT_STAT  200
#define COL_SELECTED   250

typedef enum { MODE_BUY, MODE_SELL } ShopMode;
typedef enum { SHOP_OK, SHOP_NO_GOLD, SHOP_NO_SPACE, SHOP_CANT_SELL } ShopResult;

/* ---- state ---------------------------------------------------------------- */
static bool        active = false;
static uint8_t     shop_id;
static ShopMode    mode;
static uint8_t     cursor, scroll;
static const char *message = "";

static uint8_t p_up, p_down, p_left, p_right, p_action, p_back;

/* ---- helpers -------------------------------------------------------------- */
static bool edge(uint8_t *prev, uint8_t now)
{
    bool pressed = key_just_pressed(*prev, now);
    *prev = now;
    return pressed;
}

static void list_nav(uint8_t count, uint8_t rows, uint8_t *cur, uint8_t *scr,
                     bool up, bool down)
{
    if (count == 0) { *cur = *scr = 0; return; }
    if (*cur >= count) *cur = count - 1;
    if (up && *cur > 0) (*cur)--;
    if (down && *cur < count - 1) (*cur)++;
    if (*cur < *scr) *scr = *cur;
    if (*cur >= *scr + rows) *scr = *cur - rows + 1;
}

static void draw_box(int16_t x, int16_t y, int16_t w, int16_t h)
{
    gfx_SetColor(COL_PANEL_BG);
    gfx_FillRectangle(x, y, w, h);
    gfx_SetColor(COL_BORDER);
    gfx_Rectangle(x, y, w, h);
    gfx_Rectangle(x + 2, y + 2, w - 4, h - 4);
}

/* Draws word-wrapped text; stops before max_y. Returns y below the text. */
static int draw_wrapped(const char *src, int16_t x, int16_t y, uint16_t max_w, uint16_t max_y)
{
    char line[48];

    while (src && *src && y < max_y)
    {
        uint8_t len = 0, last_space = 0;

        while (src[len] && len < sizeof(line) - 2)
        {
            if (src[len] == ' ') last_space = len;
            memcpy(line, src, len + 1);
            line[len + 1] = '\0';
            if (gfx_GetStringWidth(line) > max_w)
            {
                if (last_space > 0) len = last_space;
                break;
            }
            len++;
        }
        if (len == 0) len = 1;

        memcpy(line, src, len);
        line[len] = '\0';
        gfx_PrintStringXY(line, x, y);

        src += len;
        while (*src == ' ') src++;
        y += 12;
    }
    return y;
}

/* Number of occupied slots, counted directly rather than trusting used_slots. */
static uint8_t sell_count(const Inventory *inv)
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < INVENTORY_SIZE; i++)
        if (inv->slots[i].id != ITEM_NONE) n++;
    return n;
}

/* Maps a position in the sell list to an inventory slot. */
static InventorySlot *sell_slot_at(Inventory *inv, uint8_t n)
{
    for (uint8_t i = 0; i < INVENTORY_SIZE; i++)
    {
        if (inv->slots[i].id == ITEM_NONE) continue;
        if (n == 0) return &inv->slots[i];
        n--;
    }
    return NULL;
}

static uint8_t list_len(const Inventory *inv)
{
    return (mode == MODE_BUY) ? shop_defs[shop_id].count : sell_count(inv);
}

/* ---- transactions (no UI) ------------------------------------------------- */
static ShopResult do_buy(Inventory *inv, const ShopEntry *e)
{
    if (inv->gold < e->price)                return SHOP_NO_GOLD;
    if (!inventory_can_add(inv, e->item, 1)) return SHOP_NO_SPACE;

    inventory_remove_gold(inv, e->price);
    inventory_add(inv, e->item, 1);
    return SHOP_OK;
}

static ShopResult do_sell(Inventory *inv, ItemId id)
{
    uint16_t price = item_defs[id].value / 2;
    if (price == 0) return SHOP_CANT_SELL;

    inventory_remove(inv, id, 1);
    inventory_add_gold(inv, price);
    return SHOP_OK;
}

/* ---- public API ----------------------------------------------------------- */
void shop_open(uint8_t id)
{
    active  = true;
    shop_id = id;
    mode    = MODE_BUY;
    cursor  = scroll = 0;
    message = "Welcome!";

    /* Snapshot held keys so the 2nd press that opened the shop
     * doesn't count as a purchase on the first frame. */
    p_up     = kb_IsDown(kb_KeyUp);
    p_down   = kb_IsDown(kb_KeyDown);
    p_left   = kb_IsDown(kb_KeyLeft);
    p_right  = kb_IsDown(kb_KeyRight);
    p_action = kb_IsDown(kb_Key2nd);
    p_back   = kb_IsDown(kb_KeyAlpha) || kb_IsDown(kb_KeyClear);
}

bool shop_is_active(void) { return active; }

/* ---- drawing -------------------------------------------------------------- */
static void draw_tabs(void)
{
    static const char *labels[2] = { "BUY", "SELL" };
    int x = HEADER_X;

    for (uint8_t i = 0; i < 2; i++)
    {
        bool on = (i == (uint8_t)mode);
        gfx_SetColor(on ? COL_SELECTED : COL_PANEL_BG);
        gfx_FillRectangle(x, HEADER_Y, TAB_WIDTH, HEADER_H);
        gfx_SetColor(COL_BORDER);
        gfx_Rectangle(x, HEADER_Y, TAB_WIDTH, HEADER_H);
        gfx_SetTextFGColor(on ? COL_BG : COL_TEXT);
        gfx_PrintStringXY(labels[i], x + 4, HEADER_Y + 9);
        x += TAB_WIDTH + TAB_GAP;
    }

    gfx_SetTextFGColor(COL_TEXT);
    gfx_PrintStringXY(shop_defs[shop_id].name, x + 12, HEADER_Y + 9);
}

static void draw_list(Inventory *inv, uint8_t len)
{
    const ShopDef *def = &shop_defs[shop_id];

    draw_box(LIST_X, LIST_Y, LIST_WIDTH, ROWS_VISIBLE * ROW_HEIGHT + 10);

    if (len == 0)
    {
        gfx_SetTextFGColor(COL_TEXT_DIM);
        gfx_PrintStringXY("(nothing)", LIST_X + 14, LIST_Y + 14);
        return;
    }

    for (uint8_t row = 0; row < ROWS_VISIBLE; row++)
    {
        uint8_t idx = scroll + row;
        if (idx >= len) break;

        ItemId id;
        uint16_t price;
        if (mode == MODE_BUY)
        {
            id = def->entries[idx].item;
            price = def->entries[idx].price;
        }
        else
        {
            id = sell_slot_at(inv, idx)->id;
            price = item_defs[id].value / 2;
        }

        int y = LIST_Y + 5 + row * ROW_HEIGHT;
        bool affordable = (mode == MODE_SELL) ? (price > 0) : (inv->gold >= price);

        if (idx == cursor)
        {
            gfx_SetColor(COL_SELECTED);
            gfx_FillRectangle(LIST_X + 4, y, LIST_WIDTH - 8, ROW_HEIGHT - 2);
            gfx_SetTextFGColor(COL_BG);
            gfx_PrintStringXY(">", LIST_X + 8, y + 6);
        }
        else
        {
            gfx_SetTextFGColor(affordable ? COL_TEXT : COL_TEXT_DIM);
        }

        char buf[10];
        snprintf(buf, sizeof(buf), "%.9s", item_defs[id].name);
        gfx_PrintStringXY(buf, LIST_X + 22, y + 6);
        
        if (price > 0) sprintf(buf, "%dG", price);
        else           sprintf(buf, "--");
        gfx_PrintStringXY(buf, LIST_X + LIST_WIDTH - 50, y + 6);
    }
}

static void draw_detail(Inventory *inv, uint8_t len)
{
    draw_box(PANEL_X, PANEL_Y, PANEL_WIDTH, PANEL_HEIGHT);
    if (len == 0) return;

    ItemId id = (mode == MODE_BUY) ? shop_defs[shop_id].entries[cursor].item
                                   : sell_slot_at(inv, cursor)->id;
    const ItemDef *def = &item_defs[id];

    gfx_SetTextFGColor(COL_SELECTED);
    gfx_PrintStringXY(def->name, PANEL_X + 10, PANEL_Y + 12);

    gfx_SetTextFGColor(COL_TEXT);
    int y = draw_wrapped(def->description, PANEL_X + 10, PANEL_Y + 34,
                         PANEL_WIDTH - 20, PANEL_Y + PANEL_HEIGHT - 28);

    char stat[24] = "";
    switch (def->category)
    {
        case CATEGORY_CONSUMABLE:
            if (def->stats.consumable.heal_amount > 0)
                sprintf(stat, "Heals %d HP", def->stats.consumable.heal_amount);
            else
                sprintf(stat, "Recovers %d MP", def->stats.consumable.mp_regain);
            break;
        case CATEGORY_WEAPON: sprintf(stat, "ATK +%d", def->stats.weapon.attack); break;
        case CATEGORY_ARMOR:  sprintf(stat, "DEF +%d", def->stats.armor.defense); break;
        default: break;
    }
    if (stat[0])
    {
        gfx_SetTextFGColor(COL_TEXT_STAT);
        gfx_PrintStringXY(stat, PANEL_X + 10, y + 6);
    }

    if (mode == MODE_BUY || def->category == CATEGORY_CONSUMABLE)
    {
        char own[16];
        sprintf(own, "Owned: %d", inventory_count(inv, id));
        gfx_SetTextFGColor(COL_TEXT_DIM);
        gfx_PrintStringXY(own, PANEL_X + 10, PANEL_Y + PANEL_HEIGHT - 16);
    }
}

/* ---- the screen ----------------------------------------------------------- */
bool shop_screen(Player *player, GameState *current_state, TileMap *current_map,
                 Tile *tileset, Renderer *renderer, Camera *camera)
{
    Inventory *inv = &player->inventory;

    bool up_p     = edge(&p_up,     kb_IsDown(kb_KeyUp));
    bool down_p   = edge(&p_down,   kb_IsDown(kb_KeyDown));
    bool left_p   = edge(&p_left,   kb_IsDown(kb_KeyLeft));
    bool right_p  = edge(&p_right,  kb_IsDown(kb_KeyRight));
    bool action_p = edge(&p_action, kb_IsDown(kb_Key2nd));
    bool back_p   = edge(&p_back,   kb_IsDown(kb_KeyAlpha) || kb_IsDown(kb_KeyClear));

    /* ---- leave ---- */
    if (back_p)
    {
        active = false;
        *current_state = GAME_EXPLORATION;
        rendering_draw_initial(renderer, current_map, tileset, camera, player);
        gfx_SwapDraw();
        return false;
    }

    /* ---- switch Buy / Sell ---- */
    if ((left_p && mode == MODE_SELL) || (right_p && mode == MODE_BUY))
    {
        mode = (mode == MODE_BUY) ? MODE_SELL : MODE_BUY;
        cursor = scroll = 0;
        message = "";
    }

    uint8_t len = list_len(inv);
    list_nav(len, ROWS_VISIBLE, &cursor, &scroll, up_p, down_p);

    /* ---- buy / sell ---- */
    if (action_p && len > 0)
    {
        ShopResult r;

        if (mode == MODE_BUY)
            r = do_buy(inv, &shop_defs[shop_id].entries[cursor]);
        else
            r = do_sell(inv, sell_slot_at(inv, cursor)->id);

        switch (r)
        {
            case SHOP_OK:        message = (mode == MODE_BUY) ? "Thank you!" : "Sold."; break;
            case SHOP_NO_GOLD:   message = "Not enough gold.";  break;
            case SHOP_NO_SPACE:  message = "Inventory full.";   break;
            case SHOP_CANT_SELL: message = "Can't sell that.";  break;
        }

        /* the sell list may have shrunk; re-clamp the cursor */
        len = list_len(inv);
        list_nav(len, ROWS_VISIBLE, &cursor, &scroll, false, false);
    }

    /* ---- draw ---- */
    gfx_FillScreen(COL_BG);
    draw_tabs();
    draw_list(inv, len);
    draw_detail(inv, len);

    char g[16];
    sprintf(g, "Gold: %d", inv->gold);
    gfx_SetTextFGColor(COL_TEXT);
    gfx_PrintStringXY(g, PANEL_X + 10, PANEL_Y + PANEL_HEIGHT + 5);

    gfx_SetTextFGColor(COL_TEXT_DIM);
    gfx_PrintStringXY(mode == MODE_BUY ? "2nd: buy    Alpha: leave"
                                       : "2nd: sell   Alpha: leave",
                      LIST_X, LIST_Y + ROWS_VISIBLE * ROW_HEIGHT + 17);

    gfx_SetTextFGColor(COL_TEXT_STAT);
    gfx_PrintStringXY(message, PANEL_X + 10, PANEL_Y + PANEL_HEIGHT + 20);

    gfx_SwapDraw();
    return true;
}