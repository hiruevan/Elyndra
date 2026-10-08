#include <graphx.h>

#include "../interactions/portals.h"
#include "interactable.h"
#include "area.h"
#include "tiles.h"

// Maps
#include "maps/test_house_map.h"
#include "maps/test_map.h"
#include "maps/town_map.h"
#include "maps/world_map.h"

// Area Names
char *get_area_name(AreaId area) {
    switch (area)
    {
        case AREA_OVERWORLD:
            return "Overworld";
        case AREA_TEST:
        case AREA_TOWN:
        case AREA_HOUSE:
            return "Village";
        default:
            return "\n";
    }
}

// Area Ecounter rates
uint8_t get_encounter_rate(AreaId area) {
    switch (area)
    {
        case AREA_OVERWORLD:
            return 10;
        default:
            return 0;
    }
}

// Loading zone function
void load_area(AreaId area, TileMap *current_map)
{
    switch (area)
    {
        case AREA_OVERWORLD:
            tilemap_fullset(current_map, &world_map[0][0],
                            WORLD_MAP_WIDTH, WORLD_MAP_HEIGHT);
            break;
        case AREA_TEST:
            tilemap_fullset(current_map, &test_map[0][0],
                            TEST_MAP_WIDTH, TEST_MAP_HEIGHT);
            break;
        case AREA_TOWN:
            tilemap_fullset(current_map, &town_map[0][0],
                            TOWN_MAP_WIDTH, TOWN_MAP_HEIGHT);
            break;
        case AREA_HOUSE:
            tilemap_fullset(current_map, &test_house_map[0][0],
                            TEST_HOUSE_MAP_WIDTH, TEST_HOUSE_MAP_HEIGHT);
            break;
        default:
            break;
    }

    // Replace all chests that should be open with open chests
    Interactable *chests[MAX_AREA_CHESTS];
    uint8_t chest_count = get_area_chests(area, chests);

    for (uint8_t i = 0; i < chest_count; i++)
    {
        Interactable *chest = chests[i];

        if (chest->opened) 
            tilemap_set(current_map, chest->x, chest->y, 4);
    }
}

AreaReturn area_stack[AREA_STACK_DEPTH];
uint8_t    area_stack_top = 0;

uint8_t area_stack_push(AreaId area, uint16_t x, uint16_t y)
{
    if (area_stack_top >= AREA_STACK_DEPTH)
        return 0; /* stack full; drop the push rather than corrupt memory */

    area_stack[area_stack_top].area = area;
    area_stack[area_stack_top].x    = x;
    area_stack[area_stack_top].y    = y;
    area_stack_top++;

    return 1;
}

uint8_t area_stack_pop(AreaId *area, uint16_t *x, uint16_t *y)
{
    if (area_stack_top == 0)
        return 0; /* nothing to return to */

    area_stack_top--;
    *area = area_stack[area_stack_top].area;
    *x    = area_stack[area_stack_top].x;
    *y    = area_stack[area_stack_top].y;

    return 1;
}

const Portal *find_portal(AreaId area, uint16_t x, uint16_t y)
{
    for (uint8_t i = 0; i < PORTAL_COUNT; i++)
    {
        if (portals[i].from == area &&
            portals[i].from_x == x &&
            portals[i].from_y == y)
        {
            return &portals[i];
        }
    }

    return NULL;
}