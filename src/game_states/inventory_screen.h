#ifndef INVENTORY_SCREEN_H
#define INVENTORY_SCREEN_H
 
#include <stdbool.h>
 
#include "libs/player.h"
 
/* Draws and handles input for one frame of the Breath-of-Fire style
 * inventory menu. Call once per frame while GAME_INVENTORY is active,
 * same convention as pause_screen():
 *
 *   returns true  -> stay in GAME_INVENTORY
 *   returns false -> pop back to GAME_PAUSE_MENU
 */
bool inventory_screen(Player *player);
void inventory_screen_open(void);
 
#endif
 