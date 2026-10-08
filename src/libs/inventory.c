#include "inventory.h"
#include <string.h>

void inventory_init(Inventory *inv)
{
    memset(inv, 0, sizeof(Inventory));
}

uint8_t inventory_add(Inventory *inv, ItemId id, uint8_t count)
{
    if (id == ITEM_NONE || count == 0) return 0;
    const ItemDef *def = &item_defs[id];
    uint8_t added = 0;

    // Try to stack onto an existing slot first
    if (def->stackable) {
        for (uint8_t i = 0; i < INVENTORY_SIZE; i++) {
            InventorySlot *slot = &inv->slots[i];
            if (slot->id == id && slot->count < MAX_STACK) {
                uint8_t space = MAX_STACK - slot->count;
                uint8_t amt = (count - added < space) ? count - added : space;
                slot->count += amt;
                added += amt;
                if (added == count) return added;
            }
        }
    }

    // Fill empty slots with the remainder
    for (uint8_t i = 0; i < INVENTORY_SIZE && added < count; i++) {
        InventorySlot *slot = &inv->slots[i];
        if (slot->id == ITEM_NONE) {
            uint8_t amt = def->stackable
                ? ((count - added < MAX_STACK) ? count - added : MAX_STACK)
                : 1;
            slot->id = id;
            slot->count = amt;
            added += amt;
            inv->used_slots++;
        }
    }

    return added;
}

void inventory_add_gold(Inventory *inv, uint16_t amount) 
{
    inv->gold += amount;
}

uint8_t inventory_remove(Inventory *inv, ItemId id, uint8_t count)
{
    uint8_t removed = 0;
    for (uint8_t i = 0; i < INVENTORY_SIZE && removed < count; i++) {
        InventorySlot *slot = &inv->slots[i];
        if (slot->id == id) {
            uint8_t amt = (count - removed < slot->count) ? count - removed : slot->count;
            slot->count -= amt;
            removed += amt;
            if (slot->count == 0) {
                slot->id = ITEM_NONE;
                inv->used_slots--;
            }
        }
    }
    return removed;
}

uint8_t inventory_remove_gold(Inventory *inv, uint16_t amount)
{
    if (inv->gold < amount) return 0;
    inv->gold -= amount;
    return 1;
}

uint8_t inventory_can_add(const Inventory *inv, ItemId id, uint8_t count)
{
    const ItemDef *def = &item_defs[id];
    uint16_t room = 0;
    uint8_t empty = 0;

    for (uint8_t i = 0; i < INVENTORY_SIZE; i++)
    {
        const InventorySlot *s = &inv->slots[i];
        if (s->id == ITEM_NONE)
            empty++;
        else if (def->stackable && s->id == id)
            room += MAX_STACK - s->count;
    }

    room += (def->stackable ? MAX_STACK : 1) * empty;
    return room >= count;
}

uint8_t inventory_count(const Inventory *inv, ItemId id)
{
    uint8_t total = 0;
    for (uint8_t i = 0; i < INVENTORY_SIZE; i++)
        if (inv->slots[i].id == id) total += inv->slots[i].count;
    return total;
}

uint8_t inventory_has(const Inventory *inv, ItemId id, uint8_t count)
{
    return inventory_count(inv, id) >= count;
}