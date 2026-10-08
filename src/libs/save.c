#include "save.h"
#include "interactable.h"
#include "area.h"
#include <string.h>
#include <fileioc.h>


/* ------------------------------------------------------------------ */
/* Save file helpers                                                   */
/* ------------------------------------------------------------------ */

static void slot_appvar_name(uint8_t slot, char out[8])
{
    strcpy(out, "ELYSV");
    out[5] = (char)('0' + slot);
    out[6] = '\0';
}

// Captures live game state into a save struct (inverse of load_save)
void save_capture(GameSave *save, const Player *player, AreaId area, AreaReturn *area_stack, uint8_t area_stack_top, const Interactable *interactables, uint16_t interactable_count)
{
    save->area = area;
    memcpy(save->area_stack, area_stack, sizeof(save->area_stack));
    save->area_stack_top = area_stack_top;

    save->player_x = player->tile_x;
    save->player_y = player->tile_y;

    memcpy(save->party, player->party, sizeof(save->party));
    save->party_count = player->party_count;

    memcpy(save->inventory, player->inventory.slots, sizeof(save->inventory));
    save->gold = player->inventory.gold;

    // Pack the opened state of chests into a bitfield.
    // Clear first so stale bits from a previous save don't survive.
    memset(save->opened_chests, 0, sizeof(save->opened_chests));

    uint16_t chest_index = 0;

    for (uint16_t i = 0; i < interactable_count; i++)
    {
        if (interactables[i].type == INTERACT_CHEST ||
            interactables[i].type == INTERACT_NPC ||
            interactables[i].type == INTERACT_GOLD_STORE)
        {
            uint16_t byte_index = chest_index / 8;
            uint8_t bit_index = chest_index % 8;

            if (interactables[i].opened)
            {
                save->opened_chests[byte_index] |= (uint8_t)(1u << bit_index);
            }

            chest_index++;
        }
    }

    save->story_flag = player->story_flag;
}

// Loads a save into memory
void load_save(const GameSave *save, Player *player, Camera *camera, AreaId *area, AreaReturn *area_stack, uint8_t *area_stack_top, TileMap *map, Interactable *interactables, uint16_t interactable_count)
{
    player_init(player, save->player_x, save->player_y, save->story_flag, save->party, save->party_count);

    memcpy(player->inventory.slots, save->inventory, sizeof(player->inventory.slots));
    player->inventory.gold = save->gold;

    // Restore the opened state of chests
    uint16_t chest_index = 0;

    for (uint16_t i = 0; i < interactable_count; i++)
    {
        if (interactables[i].type == INTERACT_CHEST ||
            interactables[i].type == INTERACT_NPC ||
            interactables[i].type == INTERACT_GOLD_STORE)
        {
            uint16_t byte_index = chest_index / 8;
            uint8_t bit_index = chest_index % 8;

            interactables[i].opened = (save->opened_chests[byte_index] & (1 << bit_index)) != 0;

            chest_index++;
        }
    }

    *area = save->area;
    load_area(*area, map);
    memcpy(area_stack, save->area_stack, sizeof(save->area_stack));
    *area_stack_top = save->area_stack_top;

    camera_snap_to(camera, player->visual_x, player->visual_y, map);
    player_update_screen_position(player, camera);
    player_commit_screen_position(player);
}

// Returns true if the slot exists and holds a valid save
bool save_read(uint8_t slot, GameSave *out)
{
    char appvar[8];
    slot_appvar_name(slot, appvar);

    uint8_t handle = ti_Open(appvar, "r");
    if (!handle) return false;

    size_t read = ti_Read(out, sizeof(GameSave), 1, handle);
    ti_Close(handle);

    return read;
}

// Creates or overwrites the slot. Reuse this for saving mid-game
bool save_write(uint8_t slot, const GameSave *save)
{
    char appvar[8];
    slot_appvar_name(slot, appvar);

    uint8_t handle = ti_Open(appvar, "w");
    if (!handle) return false;

    size_t written = ti_Write(save, sizeof(GameSave), 1, handle);
    ti_Close(handle);

    return written == 1;
}

// Deletes the slot's appvar. Returns true if it was removed
// (or didn't exist to begin with)
bool save_delete(uint8_t slot)
{
    char appvar[8];
    slot_appvar_name(slot, appvar);

    ti_Delete(appvar);

    // Confirm it's really gone instead of trusting the return value
    uint8_t handle = ti_Open(appvar, "r");
    if (handle) {
        ti_Close(handle);
        return false;
    }
    return true;
}