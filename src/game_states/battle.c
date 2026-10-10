#include <graphx.h>
#include <keypadc.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "battle.h"
#include "../libs/inventory.h"
#include "../interactions/items.h"

#define C_BLACK  0
#define C_WHITE  255
#define C_BLUE   2
#define C_RED    224
#define C_GREEN  28
#define C_GRAY   181
#define C_PANEL  8

#define MAX_ENEMIES   3
#define ITEM_ROWS     5
#define MAX_BATTLE_ITEMS 16

/* ------------------------------ data ------------------------------- */

typedef struct {
    const char *name;
    int16_t hp, atk, def;
    uint16_t gold;
    uint16_t exp;
} EnemyDef;

static const EnemyDef enemy_defs[ENEMY_TYPE_COUNT] = {
    [ENEMY_SLIME]  = { "Slime",  20, 5, 1,  5,  8 },
    [ENEMY_GOBLIN] = { "Goblin", 32, 8, 2, 12, 15 },
    [ENEMY_BAT]    = { "Bat",    14, 6, 0,  7, 10 },
};

typedef struct { EnemyType type; int16_t hp; } Enemy;

typedef enum {
    PH_START_TURN,
    PH_MENU,
    PH_TARGET,
    PH_ITEM,
    PH_MESSAGE,
    PH_CHECK_PLAYER,
    PH_ENEMY,
    PH_CHECK_ENEMY,
    PH_LEVELUP,
    PH_END,
    PH_LOSE_END
} Phase;

enum { MENU_FIGHT, MENU_ITEM, MENU_GUARD, MENU_RUN, MENU_COUNT };
static const char *menu_labels[MENU_COUNT] = { "Fight", "Item", "Guard", "Run" };

static struct {
    uint8_t  active;
    Phase    phase, after_msg;
    Enemy    enemies[MAX_ENEMIES];
    uint8_t  enemy_count;
    uint8_t  member;                    /* acting / level-up index */
    uint8_t  enemy_idx;
    uint8_t  menu_sel, target_sel, item_sel;
    uint8_t  guarding[MAX_PARTY_COUNT];
    uint16_t gold_reward, exp_reward;
    ItemId   item_list[MAX_BATTLE_ITEMS];
    uint8_t  item_count;
    char     msg[48];
} B;

/* ------------------------------ input ------------------------------ */

typedef struct { uint8_t up, down, ok, back; } Input;
static uint8_t prev_keys[4];

static uint8_t edge(uint8_t idx, uint8_t now)
{
    uint8_t r = now && !prev_keys[idx];
    prev_keys[idx] = now;
    return r;
}

static Input read_input(void)
{
    Input in;
    in.up   = edge(0, kb_IsDown(kb_KeyUp)   || kb_IsDown(kb_KeyLeft));
    in.down = edge(1, kb_IsDown(kb_KeyDown) || kb_IsDown(kb_KeyRight));
    in.ok   = edge(2, kb_IsDown(kb_Key2nd));
    in.back = edge(3, kb_IsDown(kb_KeyAlpha));
    return in;
}

/* ----------------------------- helpers ----------------------------- */

static uint8_t member_alive(const Player *p, uint8_t i)
{
    return p->party[i].hp > 0;
}

static uint8_t party_alive_count(const Player *p)
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < p->party_count; i++)
        if (member_alive(p, i)) n++;
    return n;
}

static uint8_t enemies_alive_count(void)
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < B.enemy_count; i++)
        if (B.enemies[i].hp > 0) n++;
    return n;
}

static uint8_t step_target(uint8_t from, int8_t dir)
{
    uint8_t i = from;
    for (uint8_t n = 0; n < B.enemy_count; n++) {
        i = (uint8_t)((i + B.enemy_count + dir) % B.enemy_count);
        if (B.enemies[i].hp > 0) return i;
    }
    return from;
}

static int16_t calc_damage(int16_t atk, int16_t def, uint8_t guarded)
{
    int16_t dmg = atk * 2 - def + (int16_t)(rand() % 5) - 2;
    if (dmg < 1) dmg = 1;
    if (guarded) dmg = (dmg + 1) / 2;
    return dmg;
}

static void say(Phase next, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(B.msg, sizeof(B.msg), fmt, ap);
    va_end(ap);
    B.after_msg = next;
    B.phase = PH_MESSAGE;
}

static void begin_player_turn(const Player *p)
{
    B.member = 0;
    while (B.member < p->party_count && !member_alive(p, B.member))
        B.member++;
    B.menu_sel = 0;
    B.phase = PH_MENU;
}

static void next_member(const Player *p)
{
    B.member++;
    while (B.member < p->party_count && !member_alive(p, B.member))
        B.member++;

    if (B.member >= p->party_count) {
        B.enemy_idx = 0;
        B.phase = PH_ENEMY;
    } else {
        B.menu_sel = 0;
        B.phase = PH_MENU;
    }
}

/* Build a list of distinct consumables the player owns */
static void build_item_list(const Player *p)
{
    B.item_count = 0;
    for (uint8_t i = 0; i < INVENTORY_SIZE && B.item_count < MAX_BATTLE_ITEMS; i++) {
        ItemId id = p->inventory.slots[i].id;
        if (id == ITEM_NONE || p->inventory.slots[i].count == 0) continue;
        if (item_defs[id].category != CATEGORY_CONSUMABLE) continue;

        uint8_t dup = 0;
        for (uint8_t j = 0; j < B.item_count; j++)
            if (B.item_list[j] == id) { dup = 1; break; }
        if (!dup) B.item_list[B.item_count++] = id;
    }
}

// ------------- Level up --------------------
static uint16_t scaled_stat(
    uint16_t start,
    uint16_t cap,
    uint16_t level,
    uint16_t target_level
) {
    if (level <= 1)
        return start;

    if (level >= target_level)
        return cap;

    return start + (cap - start) * (level - 1)
        / (target_level - 1);
}

static void level_up(PartyMember *m)
{
    m->level++;

    // EXP requirement (keep your existing formula for now)
    m->exp_next = (uint16_t)(m->exp_next * 1.8f);
    m->exp_next += 2 * m->level;

    // Remember previous maximums so we can apply the gains.
    uint16_t old_max_hp = m->max_hp;
    uint16_t old_max_mp = m->max_mp;

    // HP: 20 -> 999 by level 45
    m->max_hp = scaled_stat(20, 999, m->level, 45);

    // MP: 8 -> 99 by level 45
    m->max_mp = scaled_stat(8, 99, m->level, 45);

    // Attack: 12 -> 99 by level 50
    m->attack = scaled_stat(12, 99, m->level, 50);

    // Defense: 16 -> 99 by level 50
    m->defense = scaled_stat(16, 99, m->level, 50);

    // Speed: 10 -> 99 by level 55
    m->speed = scaled_stat(10, 99, m->level, 55);

    // Increase current HP/MP by the amount their maximums grew.
    m->hp += m->max_hp - old_max_hp;
    m->mp += m->max_mp - old_max_mp;

    // Never exceed the maximums.
    if (m->hp > m->max_hp)
        m->hp = m->max_hp;

    if (m->mp > m->max_mp)
        m->mp = m->max_mp;
}
/* ------------------------------ public ----------------------------- */

void battle_start(EnemyType type, uint8_t count)
{
    if (count < 1) count = 1;
    if (count > MAX_ENEMIES) count = MAX_ENEMIES;

    memset(&B, 0, sizeof(B));
    B.enemy_count = count;
    for (uint8_t i = 0; i < count; i++) {
        B.enemies[i].type = type;
        B.enemies[i].hp = enemy_defs[type].hp;
        B.gold_reward += enemy_defs[type].gold;
        B.exp_reward  += enemy_defs[type].exp;
    }
    B.active = 1;

    /* Keys must be released first so the press that triggered the
     * encounter doesn't skip the intro message. */
    memset(prev_keys, 1, sizeof(prev_keys));

    say(PH_START_TURN, "%s%s appeared!", enemy_defs[type].name,
        count > 1 ? "s" : "");
}

uint8_t battle_active(void) { return B.active; }

/* ------------------------------ drawing ---------------------------- */

static void draw_bar(int x, int y, int w, int cur, int max)
{
    gfx_SetColor(C_BLACK);
    gfx_FillRectangle_NoClip(x, y, w, 4);
    if (cur > 0 && max > 0) {
        int fw = (cur * w) / max;
        if (fw < 1) fw = 1;
        gfx_SetColor((cur * 4 < max) ? C_RED : C_GREEN);
        gfx_FillRectangle_NoClip(x, y, fw, 4);
    }
}

static void draw_box(int x, int y, int w, int h)
{
    gfx_SetColor(C_BLUE);
    gfx_FillRectangle_NoClip(x, y, w, h);
    gfx_SetColor(C_WHITE);
    gfx_Rectangle_NoClip(x, y, w, h);
}

static void text(int x, int y, const char *s)
{
    gfx_SetTextFGColor(C_WHITE);
    gfx_SetTextBGColor(C_BLUE);
    gfx_SetTextTransparentColor(C_BLUE);
    gfx_PrintStringXY(s, x, y);
}

static void draw_battle(const Player *p)
{
    char buf[24];

    gfx_FillScreen(C_PANEL);

    /* Enemies (placeholder boxes: swap for sprites later) */
    for (uint8_t i = 0; i < B.enemy_count; i++) {
        const Enemy *e = &B.enemies[i];
        if (e->hp <= 0) continue;
        const EnemyDef *d = &enemy_defs[e->type];
        int x = 30 + i * 95, y = 30;

        gfx_SetColor(C_RED);
        gfx_FillRectangle_NoClip(x, y, 48, 48);
        gfx_SetColor(C_BLACK);
        gfx_Rectangle_NoClip(x, y, 48, 48);

        gfx_SetTextFGColor(C_WHITE);
        gfx_SetTextTransparentColor(C_PANEL);
        gfx_SetTextBGColor(C_PANEL);
        gfx_PrintStringXY(d->name, x, y + 52);
        draw_bar(x, y + 64, 48, e->hp, d->hp);

        if (B.phase == PH_TARGET && B.target_sel == i)
            gfx_PrintStringXY("v", x + 20, y - 12);
    }

    /* Message strip */
    if (B.phase == PH_MESSAGE) {
        draw_box(4, 118, 312, 22);
        text(10, 125, B.msg);
    }

    /* Party panel */
    draw_box(4, 144, 190, 92);
    for (uint8_t i = 0; i < p->party_count && i < MAX_PARTY_COUNT; i++) {
        const PartyMember *m = &p->party[i];
        int y = 148 + i * 12;
        if (B.phase == PH_MENU && B.member == i) text(7, y, ">");
        text(15, y, m->name);
        sprintf(buf, "%d/%d", (int)m->hp, (int)m->max_hp);
        text(62, y, buf);
        sprintf(buf, "MP%d", (int)m->mp);
        text(115, y, buf);
        draw_bar(150, y + 2, 40, m->hp, m->max_hp);
    }

    /* Command panel */
    draw_box(198, 144, 118, 92);
    if (B.phase == PH_MENU) {
        for (uint8_t i = 0; i < MENU_COUNT; i++) {
            if (B.menu_sel == i) text(204, 150 + i * 14, ">");
            text(214, 150 + i * 14, menu_labels[i]);
        }
    } else if (B.phase == PH_ITEM) {
        uint8_t top = 0;
        if (B.item_sel >= ITEM_ROWS) top = B.item_sel - ITEM_ROWS + 1;
        for (uint8_t r = 0; r < ITEM_ROWS && top + r < B.item_count; r++) {
            uint8_t idx = top + r;
            ItemId id = B.item_list[idx];
            if (B.item_sel == idx) text(204, 148 + r * 13, ">");
            sprintf(buf, "%.8s x%d", item_defs[id].name,
                    (int)inventory_count(&p->inventory, id));
            text(212, 148 + r * 13, buf);
        }
        text(204, 224, "ALPHA: back");
    } else if (B.phase == PH_TARGET) {
        text(204, 150, "Pick target");
        text(204, 224, "ALPHA: back");
    }

    gfx_SwapDraw();
}

/* ------------------------------ logic ------------------------------ */

void battle_logic(GameState *current_state, Player *player, TileMap *current_map, Tile *tileset, Renderer *renderer, Camera *camera)
{
    if (!B.active)
        battle_start((EnemyType)(rand() % ENEMY_TYPE_COUNT), 1 + rand() % 3);

    Input in = read_input();
    PartyMember *actor = &player->party[B.member < player->party_count ? B.member : 0];

    switch (B.phase)
    {
    case PH_MESSAGE:
        if (in.ok) B.phase = B.after_msg;
        break;

    case PH_START_TURN:
        begin_player_turn(player);
        break;

    case PH_MENU:
        if (in.up)   B.menu_sel = (B.menu_sel + MENU_COUNT - 1) % MENU_COUNT;
        if (in.down) B.menu_sel = (B.menu_sel + 1) % MENU_COUNT;
        if (!in.ok) break;

        switch (B.menu_sel) {
        case MENU_FIGHT:
            B.target_sel = step_target(B.enemy_count - 1, 1);
            B.phase = PH_TARGET;
            break;
        case MENU_ITEM:
            build_item_list(player);
            if (B.item_count == 0) {
                say(PH_MENU, "No usable items!");
            } else {
                B.item_sel = 0;
                B.phase = PH_ITEM;
            }
            break;
        case MENU_GUARD:
            B.guarding[B.member] = 1;
            say(PH_CHECK_PLAYER, "%s guards.", actor->name);
            break;
        case MENU_RUN:
        {
            /* Faster member = easier escape: 40% base + 3% per speed point */
            int chance = 40 + 3 * member_speed(actor);
            if (chance > 90) chance = 90;
            if (rand() % 100 < chance) say(PH_END, "Got away safely!");
            else say(PH_CHECK_PLAYER, "Couldn't escape!");
            break;
        }
        }
        break;

    case PH_TARGET:
        if (in.up)   B.target_sel = step_target(B.target_sel, -1);
        if (in.down) B.target_sel = step_target(B.target_sel, 1);
        if (in.back) { B.phase = PH_MENU; break; }
        if (in.ok) {
            Enemy *e = &B.enemies[B.target_sel];
            const EnemyDef *d = &enemy_defs[e->type];
            int16_t dmg = calc_damage((int16_t)member_attack(actor), d->def, 0);
            e->hp -= dmg;
            if (e->hp <= 0) {
                e->hp = 0;
                say(PH_CHECK_PLAYER, "%s hit %s! It fell.", actor->name, d->name);
            } else {
                say(PH_CHECK_PLAYER, "%s hit %s for %d!", actor->name, d->name, (int)dmg);
            }
        }
        break;

    case PH_ITEM:
        if (in.up && B.item_sel > 0) B.item_sel--;
        if (in.down && B.item_sel + 1 < B.item_count) B.item_sel++;
        if (in.back) { B.phase = PH_MENU; break; }
        if (in.ok) {
            ItemId id = B.item_list[B.item_sel];
            const ItemDef *def = &item_defs[id];

            if (!member_can_use(actor, def) || !status_apply_item(actor, def)) {
                say(PH_MENU, "It would have no effect.");
            } else {
                inventory_remove(&player->inventory, id, 1);
                say(PH_CHECK_PLAYER, "%s used %s!", actor->name, def->name);
            }
        }
        break;

    case PH_CHECK_PLAYER:
        if (enemies_alive_count() == 0) {
            inventory_add_gold(&player->inventory, B.gold_reward);
            for (uint8_t i = 0; i < player->party_count; i++)
                if (member_alive(player, i))
                    player->party[i].exp += B.exp_reward;
            B.member = 0;
            say(PH_LEVELUP, "Victory! +%dG +%dXP", (int)B.gold_reward, (int)B.exp_reward);
        } else {
            next_member(player);
        }
        break;

    case PH_LEVELUP:
        /* Walk through the party; announce each level gained */
        while (B.member < player->party_count) {
            PartyMember *m = &player->party[B.member];
            if (member_alive(player, B.member) && m->exp >= m->exp_next) {
                level_up(m);
                say(PH_LEVELUP, "%s reached Lv%d!", m->name, (int)m->level);
                break;
            }
            B.member++;
        }
        if (B.member >= player->party_count) B.phase = PH_END;
        break;

    case PH_ENEMY:
    {
        while (B.enemy_idx < B.enemy_count && B.enemies[B.enemy_idx].hp <= 0)
            B.enemy_idx++;

        if (B.enemy_idx >= B.enemy_count) {
            memset(B.guarding, 0, sizeof(B.guarding));
            begin_player_turn(player);
            break;
        }

        uint8_t pick = (uint8_t)(rand() % party_alive_count(player));
        uint8_t t = 0;
        for (uint8_t i = 0; i < player->party_count; i++) {
            if (!member_alive(player, i)) continue;
            if (pick-- == 0) { t = i; break; }
        }

        const EnemyDef *d = &enemy_defs[B.enemies[B.enemy_idx].type];
        PartyMember *victim = &player->party[t];
        int16_t dmg = calc_damage(d->atk, (int16_t)member_defense(victim), B.guarding[t]);

        if (victim->hp <= (uint16_t)dmg) victim->hp = 0;
        else victim->hp -= dmg;

        if (victim->hp == 0)
            say(PH_CHECK_ENEMY, "%s hit %s! %s fell.", d->name, victim->name, victim->name);
        else
            say(PH_CHECK_ENEMY, "%s hit %s for %d!", d->name, victim->name, (int)dmg);
        break;
    }

    case PH_CHECK_ENEMY:
        if (party_alive_count(player) == 0) {
            say(PH_LOSE_END, "The party was wiped out...");
        } else {
            B.enemy_idx++;
            B.phase = PH_ENEMY;
        }
        break;

    case PH_END:
        B.active = 0;
        rendering_draw_initial(renderer, current_map, tileset, camera, player);
        gfx_SwapDraw();
        *current_state = GAME_EXPLORATION;
        return;

    case PH_LOSE_END:
        // TODO: Replace with real game-over ---------------------------------------
        for (uint8_t i = 0; i < player->party_count; i++) {
            PartyMember *m = &player->party[i];
            m->hp = m->max_hp / 2 > 0 ? m->max_hp / 2 : 1;
        }
        B.active = 0;
        *current_state = GAME_TITLE;
        return;
    }

    draw_battle(player);
}