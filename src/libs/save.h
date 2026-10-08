#pragma once
#include <graphx.h>
#include "player.h"
#include "area.h"
#include "interactable.h"

// Player start pos
#define START_X 10
#define START_Y 10

// Save edits
#define NAME_LEN     5
#define MAX_SAVES    3

typedef struct
{
    char name[NAME_LEN + 1];      // +1 for the '\0'

    AreaId  area;
    AreaReturn area_stack[AREA_STACK_DEPTH];
    uint8_t area_stack_top;

    uint16_t player_x;
    uint16_t player_y;

    PartyMember party[MAX_PARTY_COUNT];
    uint8_t party_count;

    InventorySlot inventory[INVENTORY_SIZE];
    uint16_t gold;

    uint8_t opened_chests[32];    // bitfield: 256 chests
    uint8_t story_flag;           // story track int
} GameSave;

void save_capture(GameSave *save, const Player *player, AreaId area, AreaReturn *area_stack, uint8_t area_stack_top, const Interactable *interactables, uint16_t interactable_count);
void load_save(const GameSave *save, Player *player, Camera *camera, AreaId *area, AreaReturn *area_stack, uint8_t *area_stack_top, TileMap *map, Interactable *interactables, uint16_t interactable_count);
bool save_delete(uint8_t slot);
bool save_read(uint8_t slot, GameSave *out);
bool save_write(uint8_t slot, const GameSave *save);