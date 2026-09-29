#include "title.h"
#include "libs/rendering.h"
#include "libs/player.h"
#include "libs/tiles.h"
#include "game_state.h"

// Title
void title_logic(Renderer *renderer, Camera *camera, Player *player, uint8_t action_key_down, GameState *current_state, TileMap *current_map, Tile *tileset)
{
    // Clear the screen
    gfx_FillScreen(1);
    gfx_SetTextFGColor(223);

    // Title
    gfx_SetTextScale(2, 2);
    gfx_SetTextXY(100, 60);
    gfx_PrintString("ELYNDRA");

    // Subtitle
    gfx_SetTextScale(1, 1);
    gfx_SetTextXY(95, 100);
    gfx_PrintString("A TI-84 Adventure");

    // Start prompt
    gfx_SetTextXY(95, 180);
    gfx_PrintString("Press 2nd to Start");

    // Check for start
    if (action_key_down) {
        *current_state = GAME_EXPLORATION;
        rendering_draw_initial(renderer, current_map, tileset, camera, player); // Initialize First Frame
    }

    gfx_SwapDraw();
}