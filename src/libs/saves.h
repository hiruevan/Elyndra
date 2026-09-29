#include <graphx.h>

typedef struct
{
    uint8_t area;

    uint16_t player_x;
    uint16_t player_y;

    uint16_t hp;
    uint16_t mp; // etc

    uint8_t opened_chests[32];

    uint8_t flags[32];
} GameSave;