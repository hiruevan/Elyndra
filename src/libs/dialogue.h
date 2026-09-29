#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stdint.h>

#define MAX_DIALOGUE_LINES 8

typedef struct
{
    const char *lines[MAX_DIALOGUE_LINES];
    uint8_t line_count;
} Dialogue;

void start_dialogue(uint8_t dialogue_id);
void dialogue_update(uint8_t action_pressed);
uint8_t dialogue_is_active(void);
void dialogue_close(void);

#endif