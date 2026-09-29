#include <graphx.h>
#include <keypadc.h>
#include <string.h>
#include "text_engine.h"

#define TEXTBOX_MAX_LEN    96
#define TEXTBOX_MAX_LINES  4
#define TEXTBOX_PAD_X      6
#define TEXTBOX_PAD_Y      6
#define TEXTBOX_LINE_H     10

static TextBox textbox = { 0 };

void textbox_show(const char *message)
{
    uint8_t i = 0;

    while (message[i] != '\0' && i < TEXTBOX_MAX_LEN - 1)
    {
        textbox.message[i] = message[i];
        i++;
    }
    textbox.message[i] = '\0';

    textbox.active = 1;
}

void textbox_close(void)
{
    textbox.active = 0;
}

/* Breaks textbox.message into lines that fit within max_width pixels,
   wrapping on word boundaries (hard-breaking a word only if it alone
   is wider than the box). Returns the number of lines produced. */
static uint8_t textbox_wrap(char lines[][TEXTBOX_MAX_LEN], uint8_t max_lines, unsigned int max_width)
{
    uint8_t line_count = 0;
    uint16_t pos = 0;

    while (textbox.message[pos] != '\0' && line_count < max_lines)
    {
        uint16_t line_start = pos;
        uint16_t last_space = 0;
        uint16_t line_len;

        while (textbox.message[pos] != '\0')
        {
            char saved;

            if (textbox.message[pos] == ' ')
                last_space = pos;

            /* temporarily cap the string so we can measure just this slice */
            saved = textbox.message[pos + 1];
            textbox.message[pos + 1] = '\0';
            {
                unsigned int w = gfx_GetStringWidth(&textbox.message[line_start]);
                textbox.message[pos + 1] = saved;
                if (w > max_width)
                    break;
            }
            pos++;
        }

        if (textbox.message[pos] == '\0')
        {
            /* remainder of the string fits on this line */
            line_len = pos - line_start;
            strncpy(lines[line_count], &textbox.message[line_start], line_len);
            lines[line_count][line_len] = '\0';
            line_count++;
            break;
        }

        if (last_space > line_start)
        {
            /* wrap at the last space before the overflow point */
            line_len = last_space - line_start;
            pos = last_space + 1; /* skip the space */
        }
        else
        {
            /* single word is wider than the box: hard-break it */
            line_len = pos - line_start;
            if (line_len == 0)
            {
                line_len = 1;
                pos = line_start + 1;
            }
        }

        strncpy(lines[line_count], &textbox.message[line_start], line_len);
        lines[line_count][line_len] = '\0';
        line_count++;
    }

    return line_count;
}

void textbox_draw(void)
{
    char lines[TEXTBOX_MAX_LINES][TEXTBOX_MAX_LEN];
    uint8_t line_count, i;

    unsigned int box_x = 8;
    unsigned int box_y = GFX_LCD_HEIGHT - 48;
    unsigned int box_w = GFX_LCD_WIDTH - 16;
    unsigned int box_h = 40;

    unsigned int text_x   = box_x + TEXTBOX_PAD_X;
    unsigned int text_y   = box_y + TEXTBOX_PAD_Y;
    unsigned int max_w    = box_w - (TEXTBOX_PAD_X * 2);
    uint8_t max_lines_fit = (box_h - (TEXTBOX_PAD_Y * 2)) / TEXTBOX_LINE_H;

    if (max_lines_fit > TEXTBOX_MAX_LINES)
        max_lines_fit = TEXTBOX_MAX_LINES;
    if (max_lines_fit == 0)
        max_lines_fit = 1;

    gfx_SetColor(1);
    gfx_FillRectangle_NoClip(box_x, box_y, box_w, box_h);

    gfx_SetColor(3);
    gfx_Rectangle_NoClip(box_x, box_y, box_w, box_h);

    gfx_SetTextFGColor(223);

    line_count = textbox_wrap(lines, max_lines_fit, max_w);

    for (i = 0; i < line_count; i++)
    {
        gfx_PrintStringXY(lines[i], text_x, text_y + (i * TEXTBOX_LINE_H));
    }
}

uint8_t textbox_is_active(void)
{
    return textbox.active;
}