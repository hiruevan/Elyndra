#include "sprite_utils.h"

void setup_xlibc_palette(void)
{
    for (int i = 0; i < 256; i++)
    {
        uint8_t r = (i >> 5) & 7;
        uint8_t g = (i >> 2) & 7;
        uint8_t b = i & 3;

        /* Expand 3-bit channels to 5-bit */
        r = (r << 2) | (r >> 1);
        g = (g << 2) | (g >> 1);

        /* Expand 2-bit channel to 5-bit */
        b = (b << 3) | (b << 1) | (b >> 1);

        gfx_palette[i] =
            (r << 10) |
            (g << 5) |
            b;
    }
}