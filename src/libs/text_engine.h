#pragma once
/*
 * Text box engine -----------------------------------------------------
 *
 * Shows a single message in a box at the bottom of the screen. While
 * active, the main loop skips normal gameplay input and only watches
 * for the action key to dismiss it.
 */
#define TEXTBOX_MAX_LEN 96
 
typedef struct
{
    uint8_t active;
    char    message[TEXTBOX_MAX_LEN];
} TextBox;
 
void textbox_show(const char *message);

void textbox_close(void);
 
void textbox_draw(void);

uint8_t textbox_is_active(void);