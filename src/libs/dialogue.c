#include <stdint.h>

#include "dialogue.h"
#include "text_engine.h"
#include "../interactions/dialogues.h"

static uint8_t current_dialogue = 0;
static uint8_t current_line = 0;
static uint8_t dialogue_active = 0;

void start_dialogue(uint8_t dialogue_id)
{
    if (dialogue_id >= sizeof(dialogues) / sizeof(dialogues[0]))
        return;

    current_dialogue = dialogue_id;
    current_line = 0;
    dialogue_active = 1;

    textbox_show(dialogues[current_dialogue].lines[current_line]);
}

void dialogue_update(uint8_t action_pressed)
{
    if (!dialogue_active)
        return;

    if (!action_pressed)
        return;

    current_line++;

    if (current_line >= dialogues[current_dialogue].line_count)
    {
        dialogue_close();
        return;
    }

    textbox_show(
        dialogues[current_dialogue].lines[current_line]
    );
}

void dialogue_close(void)
{
    dialogue_active = 0;
    textbox_close();
}

uint8_t dialogue_is_active(void)
{
    return dialogue_active;
}