#pragma once
#include <graphx.h>

#include "tiles.h"

typedef enum 
{
    AREA_OVERWORLD,
    AREA_TOWN,
    AREA_TEST,
    AREA_HOUSE,

    AREA_COUNT
} AreaId;

#define AREA_STACK_DEPTH 32

typedef struct
{
    AreaId   area;
    uint16_t x;
    uint16_t y;
} AreaReturn;

typedef struct
{
    AreaId from;
    uint16_t from_x;
    uint16_t from_y;

    AreaId to;
    uint16_t to_x;
    uint16_t to_y;
} Portal;

uint8_t area_stack_push(AreaId area, uint16_t x, uint16_t y);
uint8_t area_stack_pop(AreaId *area, uint16_t *x, uint16_t *y);
void load_area(AreaId area, TileMap *current_map);

const Portal *find_portal(AreaId area, uint16_t x, uint16_t y);