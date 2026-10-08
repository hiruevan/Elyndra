#pragma once
#include "../libs/dialogue.h"
#include "items.h"

/* "\a" is a dot */

// Conditions
#define ALWAYS            { COND_NONE, 0 }
#define IF_FLAG_GT(n)     { COND_FLAG_GREATER, (n) }
#define IF_FLAG_LT(n)     { COND_FLAG_LESS, (n) }
#define IF_FLAG_EQ(n)     { COND_FLAG_EQUAL, (n) }
#define IF_HAS(item)      { COND_HAS_ITEM, (item) }
#define IF_LACKS(item)    { COND_NOT_HAS_ITEM, (item) }

// Actions
#define DO_NOTHING      { ACT_NONE, 0 }
#define DO_SET_FLAG(n)  { ACT_SET_FLAG, (n) }
#define DO_RAISE_FLAG(n){ ACT_RAISE_FLAG, (n) }
#define DO_GIVE(item)   { ACT_GIVE_ITEM, (item) }
#define DO_TAKE(item)   { ACT_TAKE_ITEM, (item) }

// Line funcs
#define LINE(text)                 { (text), ALWAYS, DO_NOTHING }
#define LINE_IF(cond, text)        { (text), cond,   DO_NOTHING }
#define LINE_DO(cond, text, act)   { (text), cond,   act }

static const DialogueLine elder_lines[] =
{
    LINE_IF(IF_FLAG_EQ(0), "Welcome to Elyndra adventurer."),
    LINE_IF(IF_FLAG_EQ(0), "This world is dangerous, be careful."),
    LINE_DO(IF_FLAG_EQ(0), "Take this Tonic.", DO_GIVE(ITEM_TONIC)),
    LINE_DO(IF_FLAG_EQ(0), "You will need to buy some equipment, open the chest in the room above.", DO_RAISE_FLAG(1)),

    LINE_IF(IF_HAS(ITEM_STONE_TABLET),   "That tablet... where did you find it?"),
    LINE_IF(IF_LACKS(ITEM_STONE_TABLET), "There are rumors of hidden chests around."),

    LINE_IF(IF_FLAG_GT(0), "I have done all I can to help you."),

    LINE("Good luck out there.")
};

static const Dialogue dialogues[] =
{
    { elder_lines, sizeof(elder_lines) / sizeof(elder_lines[0]) },
};