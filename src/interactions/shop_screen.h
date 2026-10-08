#pragma once
#include "../libs/player.h"

uint8_t shop_is_active(void);
void shop_update(Player *player);
void shop_draw(const Player *player);