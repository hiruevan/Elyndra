#include <stdint.h>

#include "dialogue.h"
#include "text_engine.h"
#include "../interactions/dialogues.h"
#include "../libs/player.h"

static uint8_t current_dialogue = 0;
static uint8_t current_line = 0;
static uint8_t dialogue_active = 0;
static Player *current_player = 0;

static uint8_t condition_met(const Condition *c, const Player *p)
{
    switch (c->type)
    {
        case COND_NONE:         return 1;
        case COND_FLAG_GREATER: return p->story_flag >  c->value;
        case COND_FLAG_LESS:    return p->story_flag <  c->value;
        case COND_FLAG_EQUAL:   return p->story_flag == c->value;
        case COND_HAS_ITEM:     return  player_has_item(p, c->value);
        case COND_NOT_HAS_ITEM: return !player_has_item(p, c->value);
    }
    return 0;
}

static void run_action(const Action *a, Player *p)
{
    switch (a->type)
    {
        case ACT_NONE:
            break;
        case ACT_SET_FLAG:
            p->story_flag = a->value;
            break;
        case ACT_RAISE_FLAG:
            if (p->story_flag < a->value)
                p->story_flag = a->value;
            break;
        case ACT_GIVE_ITEM:
            player_give_item(p, a->value);
            break;
        case ACT_TAKE_ITEM:
            player_take_item(p, a->value);
            break;
    }
}

static uint8_t find_next_line(uint8_t from)
{
    const Dialogue *d = &dialogues[current_dialogue];

    for (uint8_t i = from; i < d->line_count; i++)
    {
        if (condition_met(&d->lines[i].condition, current_player))
            return i;
    }
    return d->line_count;
}

static void show_current_line(void)
{
    const DialogueLine *line = &dialogues[current_dialogue].lines[current_line];

    run_action(&line->action, current_player);
    textbox_show(line->text);
}

void start_dialogue(uint8_t dialogue_id, Player *player)
{
    if (dialogue_id >= sizeof(dialogues) / sizeof(dialogues[0]))
        return;

    current_dialogue = dialogue_id;
    current_player   = player;
    current_line     = find_next_line(0);

    if (current_line >= dialogues[current_dialogue].line_count)
        return;

    dialogue_active = 1;
    show_current_line();
}

void dialogue_update(uint8_t action_pressed)
{
    if (!dialogue_active || !action_pressed)
        return;

    current_line = find_next_line(current_line + 1);

    if (current_line >= dialogues[current_dialogue].line_count)
    {
        dialogue_close();
        return;
    }

    show_current_line();
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