#ifndef BATTLE_H
#define BATTLE_H

#include <stdint.h>
#include "game_state.h"
#include "../libs/player.h"
#include "../libs/rendering.h"

typedef enum {
    ENEMY_SLIME,
    ENEMY_GOBLIN,
    ENEMY_BAT,
    ENEMY_TYPE_COUNT
} EnemyType;

/* Start a battle against `count` (1-3) enemies of one type.
 * Call this from exploration (e.g. random encounter / NPC), then set
 * current_state = GAME_BATTLE. */
void battle_start(EnemyType type, uint8_t count);

/* Returns 1 while a battle is running. */
uint8_t battle_active(void);

/* Call once per frame while current_state == GAME_BATTLE.
 * If no battle was started, a random encounter is started automatically. */
void battle_logic(GameState *current_state, Player *player, TileMap *current_map, Tile *tileset, Renderer *renderer, Camera *camera);

#endif