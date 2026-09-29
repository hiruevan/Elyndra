#pragma once
#include "area.h"
#include "tiles.h"
#include "player.h"

typedef enum
{
    INTERACT_NPC,
    INTERACT_CHEST,
    INTERACT_GOLD_STORE,
    INTERACT_SIGN,

    INTERACT_TYPE_COUNT
} InteractableType;

#define MAX_AREA_CHESTS 32

typedef struct
{
    AreaId           area;
    uint16_t         x;
    uint16_t         y;
 
    InteractableType type;
 
    /*
     * Meaning depends on `type`:
     *   INTERACT_NPC   -> index into a dialogue table (not implemented yet)
     *   INTERACT_CHEST -> index into an item table (not implemented yet)
     *   INTERACT_SIGN  -> index into a strings table (not implemented yet)
     */
    uint16_t         data;
 
    uint8_t           opened; /* used by chests so they don't re-trigger */
} Interactable;

Interactable *find_interactable(AreaId area, uint16_t x, uint16_t y);
void handle_interact(Interactable *interactable, TileMap *map, Player *player);
uint8_t get_area_chests(
    AreaId area,
    Interactable **chests
);