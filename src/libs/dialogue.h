#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stdint.h>
#include "player.h"

#define MAX_DIALOGUE_LINES 8

typedef enum {
    COND_NONE,
    COND_FLAG_GREATER,
    COND_FLAG_LESS,
    COND_FLAG_EQUAL,
    COND_HAS_ITEM,
    COND_NOT_HAS_ITEM
} ConditionType;

typedef enum {
    ACT_NONE = 0,
    ACT_SET_FLAG,
    ACT_RAISE_FLAG,
    ACT_GIVE_ITEM,
    ACT_TAKE_ITEM
} ActionType;

typedef struct {
    ConditionType type;
    uint8_t value;
} Condition;

typedef struct {
    ActionType type;
    uint8_t value;
} Action;

typedef struct {
    const char *text;
    Condition condition;
    Action action;
} DialogueLine;

typedef struct {
    const DialogueLine *lines;
    uint8_t line_count;
} Dialogue;

void start_dialogue(uint8_t dialogue_id, Player *player);
void dialogue_update(uint8_t action_pressed);
uint8_t dialogue_is_active(void);
void dialogue_close(void);

#endif