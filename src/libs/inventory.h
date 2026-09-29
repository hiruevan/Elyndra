#ifndef INVENTORY_H
#define INVENTORY_H

#include "../interactions/items.h"

#define INVENTORY_SIZE 32
#define MAX_STACK 9

typedef struct {
    ItemId  id;
    uint8_t count;
} InventorySlot;

typedef struct {
    InventorySlot slots[INVENTORY_SIZE];
    uint16_t gold;
    uint8_t used_slots;
} Inventory;

void    inventory_init(Inventory *inv);
uint8_t inventory_add(Inventory *inv, ItemId id, uint8_t count); 
uint8_t inventory_remove(Inventory *inv, ItemId id, uint8_t count);
void inventory_add_gold(Inventory *inv, uint16_t amount);
uint8_t inventory_count(const Inventory *inv, ItemId id);
uint8_t inventory_has(const Inventory *inv, ItemId id, uint8_t count);

#endif