#include <stdio.h>

#include "interactable.h"
#include "../interactions/interactables.h"
#include "../interactions/sign_strings.h"
#include "../interactions/items.h"
#include "dialogue.h"
#include "text_engine.h"
#include "tiles.h"
#include "player.h"
#include "inventory.h"

Interactable *find_interactable(AreaId area, uint16_t x, uint16_t y)
{
    for (uint8_t i = 0; i < INTERACTABLE_COUNT; i++)
    {
        if (interactables[i].area == area &&
            interactables[i].x == x &&
            interactables[i].y == y)
        {
            return &interactables[i];
        }
        
    }

    return NULL;
}


void handle_interact(Interactable *interactable, TileMap *map, Player *player)
{
    switch (interactable->type)
    {
        case INTERACT_NPC:
            start_dialogue(interactable->data);
            break;
 
        case INTERACT_CHEST:
        case INTERACT_GOLD_STORE:
            if (!interactable->opened)
            {
                interactable->opened = 1;
                tilemap_set(map, interactable->x, interactable->y, 4);
                char msg[64];
                if (interactable->type == INTERACT_CHEST) {
                    inventory_add(&player->inventory, interactable->data, 1);
                    sprintf(msg, "Obtained %s.", item_defs[interactable->data].name);
                } else {
                    inventory_add_gold(&player->inventory, interactable->data);
                    sprintf(msg, "Obtained %d gold.", interactable->data);
                }
                textbox_show(msg);
            } else {
                textbox_show("It is empty.");
            }
            break;
 
        case INTERACT_SIGN:
            textbox_show(sign_strings[interactable->data]);
            break;
 
        default:
            break;
    }
}

uint8_t get_area_chests(
    AreaId area,
    Interactable **chests
)
{
    uint8_t chest_count = 0;

    for (uint8_t i = 0; i < INTERACTABLE_COUNT; i++)
    {
        Interactable *interactable = &interactables[i];

        if (interactable->area != area)
            continue;

        if (interactable->type != INTERACT_CHEST && interactable->type != INTERACT_GOLD_STORE)
            continue;

        if (chest_count >= MAX_AREA_CHESTS)
            break;

        chests[chest_count] = interactable;
        chest_count++;
    }

    return chest_count;
}