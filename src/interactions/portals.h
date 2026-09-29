#pragma once
#include "../libs/area.h"

static const Portal portals[] =
{
    { AREA_TEST, 20, 22, AREA_HOUSE, 9, 14 },

    { AREA_OVERWORLD, 13, 9,  AREA_TEST, 0, 15 },
    { AREA_OVERWORLD, 14, 9,  AREA_TEST, 17, 39 },

    { AREA_OVERWORLD, 21, 18, AREA_TOWN, 20, 39 },
};

#define PORTAL_COUNT (sizeof(portals) / sizeof(portals[0]))