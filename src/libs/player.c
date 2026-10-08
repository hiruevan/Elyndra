#include <keypadc.h>
#include <string.h>

#include "player.h"
#include "inventory.h"

/* ---- item logic ---------------------------------------------------------- */

/* Who can use this item? Currently: living members below max HP.
 * Extend here for revive items, MP restore, etc. */
bool member_can_use(const PartyMember *m, const ItemDef *def)
{
    if (def->category != CATEGORY_CONSUMABLE)
        return false;
    if (def->stats.consumable.heal_amount > 0)
        return m->hp > 0 && m->hp < m->max_hp;
    else
        return m->mp > 0 && m->mp < m->max_mp;
}

ItemId item_id_of(const ItemDef *def)
{
    return (ItemId)(def - item_defs);
}

bool unequip_item(Inventory *inv, ItemId *slot)
{
    if (*slot == ITEM_NONE)
        return false;
    if (!inventory_add(inv, *slot, 1))   /* inventory full */
        return false;
    *slot = ITEM_NONE;
    return true;
}

bool member_can_equip(const PartyMember *m, const ItemDef *def)
{
    ItemId id = item_id_of(def);
    if (def->category == CATEGORY_WEAPON)
        return m->weapon != id;
    if (def->category == CATEGORY_ARMOR)
        return m->armor != id;
    return false;
    /* Extend here: class restrictions, level requirements, etc. */
}

bool member_can_target(const PartyMember *m, const ItemDef *def)
{
    switch (def->category)
    {
        case CATEGORY_CONSUMABLE: return member_can_use(m, def);
        case CATEGORY_WEAPON:
        case CATEGORY_ARMOR:      return member_can_equip(m, def);
        default:                  return false;
    }
}

uint16_t member_attack(const PartyMember *m)
{
    uint16_t atk = m->attack;
    if (m->weapon != ITEM_NONE)
        atk += item_defs[m->weapon].stats.weapon.attack;
    return atk;
}

uint16_t member_defense(const PartyMember *m)
{
    uint16_t def = m->defense;
    if (m->armor != ITEM_NONE)
        def += item_defs[m->armor].stats.armor.defense;
    return def;
}

int speed_with(const PartyMember *m, ItemId w, ItemId a)
{
    int spd = m->speed;
    if (w != ITEM_NONE) spd -= item_defs[w].stats.weapon.weight;
    if (a != ITEM_NONE) spd -= item_defs[a].stats.armor.weight;
    return spd < 1 ? 1 : spd;
}

int member_speed(const PartyMember *m)
{
    return speed_with(m, m->weapon, m->armor);
}

bool status_apply_item(PartyMember *m, const ItemDef *def)
{
    if (!member_can_use(m, def))
        return false;

    uint16_t heal = def->stats.consumable.heal_amount;
    uint16_t recover = def->stats.consumable.mp_regain;
    uint16_t room_h = m->max_hp - m->hp;
    uint16_t room_m = m->max_mp - m->mp;
    m->hp += (heal > room_h) ? room_h : heal;
    m->mp += (recover > room_m) ? room_m : recover;
    return true;
}

bool equip_item(Inventory *inv, PartyMember *m, ItemId id)
{
    ItemId *slot = (item_defs[id].category == CATEGORY_WEAPON) ? &m->weapon
                                                               : &m->armor;
    ItemId old = *slot;

    if (inventory_remove(inv, id, 1) == 0)
        return false;

    if (old != ITEM_NONE && !inventory_add(inv, old, 1))
    {
        inventory_add(inv, id, 1);   /* roll back, inventory was full */
        return false;
    }

    *slot = id;
    return true;
}

static const uint8_t tile_walkable[MAX_TILES] =
{
    [2]  = 1,
    [7]  = 1,
    [8]  = 1,
    [9]  = 1,
    [10]  = 1,
    [11] = 1,
    [12] = 1,
    [36] = 1
};

uint8_t player_has_item(const Player *p, uint8_t item_id)
{
    for (uint8_t i = 0; i < INVENTORY_SIZE; i++)
    {
        if (p->inventory.slots[i].id == item_id && p->inventory.slots[i].count > 0)
            return 1;
    }
    return 0;
}

uint8_t player_give_item(Player *p, uint8_t item_id)
{
    return inventory_add(&p->inventory, item_id, 1);
}

uint8_t player_take_item(Player *p, uint8_t item_id)
{
    return inventory_remove(&p->inventory, item_id, 1);
}

void player_init(
    Player *player,
    uint16_t tile_x,
    uint16_t tile_y,
    uint8_t story_flag,
    const PartyMember *party,
    uint8_t party_count
)
{
    player->tile_x = tile_x;
    player->tile_y = tile_y;

    player->visual_x = tile_x * TILE_SIZE;
    player->visual_y = tile_y * TILE_SIZE;

    player->target_x = player->visual_x;
    player->target_y = player->visual_y;

    player->screen_x =
        player->visual_x + 2;

    player->screen_y =
        player->visual_y;

    player->old_screen_x =
        player->screen_x;

    player->old_screen_y =
        player->screen_y;

    player->moving = 0;
    player->facing = DIRECTION_DOWN;

    player->story_flag = story_flag;

    inventory_init(&player->inventory);

    if (party_count > 7)
        party_count = 7;

    memcpy(player->party, party, party_count * sizeof(PartyMember));
    player->party_count = party_count;
}

uint8_t player_can_move_to(
    const TileMap *world,
    uint16_t x,
    uint16_t y
)
{
    if (x >= world->width || y >= world->height)
        return 0;

    return tile_walkable[tilemap_get(world, x, y)] != 0;
}


void player_warp(
    Player *player,
    uint16_t tile_x,
    uint16_t tile_y
)
{
    player->tile_x = tile_x;
    player->tile_y = tile_y;

    player->visual_x = tile_x * TILE_SIZE;
    player->visual_y = tile_y * TILE_SIZE;

    player->target_x = player->visual_x;
    player->target_y = player->visual_y;

    player->moving = 0;
}

void player_handle_input(
    Player *player,
    const TileMap *world
)
{
    int8_t move_x = 0;
    int8_t move_y = 0;

    /*
     * Don't accept another movement while
     * the player is already moving.
     */
    if (player->moving)
        return;

    /*
     * Match the original movement priority:
     *
     * Right
     * Left
     * Down
     * Up
     */
    if (kb_IsDown(kb_KeyRight)) {
        move_x = 1;
        player->facing = DIRECTION_RIGHT;
    }
    else if (kb_IsDown(kb_KeyLeft)) {
        move_x = -1;
        player->facing = DIRECTION_LEFT;
    }
    else if (kb_IsDown(kb_KeyDown)) {
        move_y = 1;
        player->facing = DIRECTION_DOWN;
    }
    else if (kb_IsDown(kb_KeyUp)) {
        move_y = -1;
        player->facing = DIRECTION_UP;
    }

    if (!move_x && !move_y)
        return;

    /*
     * Calculate the destination tile.
     */
    int16_t target_x = player->tile_x + move_x;
    int16_t target_y = player->tile_y + move_y;

    /*
     * Prevent negative coordinates.
     */
    if (target_x < 0 || target_y < 0)
        return;

    /*
     * Check collision.
     */
    if (!player_can_move_to(
            world,
            target_x,
            target_y))
    {
        return;
    }

    /*
     * Update the logical grid position.
     */
    player->tile_x = target_x;
    player->tile_y = target_y;

    /*
     * Set the smooth movement target.
     */
    player->target_x = target_x * TILE_SIZE;
    player->target_y = target_y * TILE_SIZE;

    player->moving = 1;
}


void player_update(
    Player *player
)
{
    /*
     * Move visual X toward target X.
     */
    if (player->visual_x < player->target_x)
    {
        player->visual_x += PLAYER_SPEED;

        if (player->visual_x > player->target_x)
            player->visual_x = player->target_x;
    }
    else if (player->visual_x > player->target_x)
    {
        player->visual_x -= PLAYER_SPEED;

        if (player->visual_x < player->target_x)
            player->visual_x = player->target_x;
    }

    /*
     * Move visual Y toward target Y.
     */
    if (player->visual_y < player->target_y)
    {
        player->visual_y += PLAYER_SPEED;

        if (player->visual_y > player->target_y)
            player->visual_y = player->target_y;
    }
    else if (player->visual_y > player->target_y)
    {
        player->visual_y -= PLAYER_SPEED;

        if (player->visual_y < player->target_y)
            player->visual_y = player->target_y;
    }

    /*
     * Movement is finished once the visual position
     * reaches the target.
     */
    if (player->visual_x == player->target_x &&
        player->visual_y == player->target_y)
    {
        player->moving = 0;
    }
}


void player_update_screen_position(
    Player *player,
    const Camera *camera
)
{
    int16_t camera_pixel_x =
        camera->x * TILE_SIZE + camera->pixel_x;

    int16_t camera_pixel_y =
        camera->y * TILE_SIZE + camera->pixel_y;

    /*
     * Keep the original +2 X offset.
     */
    player->screen_x =
        player->visual_x - camera_pixel_x + 2;

    player->screen_y =
        player->visual_y - camera_pixel_y;
}


uint8_t player_screen_changed(
    const Player *player
)
{
    return (
        player->screen_x != player->old_screen_x ||
        player->screen_y != player->old_screen_y
    );
}


void player_commit_screen_position(
    Player *player
)
{
    player->old_screen_x = player->screen_x;
    player->old_screen_y = player->screen_y;
}

void get_facing_tile(
    const Player *player,
    uint16_t *x,
    uint16_t *y
)
{
    *x = player->tile_x;
    *y = player->tile_y;

    switch (player->facing)
    {
        case DIRECTION_UP:
            if (*y > 0)
                (*y)--;
            break;

        case DIRECTION_DOWN:
            (*y)++;
            break;

        case DIRECTION_LEFT:
            if (*x > 0)
                (*x)--;
            break;

        case DIRECTION_RIGHT:
            (*x)++;
            break;
    }
}