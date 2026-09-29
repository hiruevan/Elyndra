#pragma once
#include <graphx.h>

// Key just pressed helper function
static uint8_t key_just_pressed(uint8_t was_down_last_frame, uint8_t is_down_now)
{
    return is_down_now && !was_down_last_frame;
}