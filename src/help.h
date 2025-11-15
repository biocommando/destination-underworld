#pragma once
#include "allegro5/allegro.h"

/**
 * Displays the help file from <mission pack datadir>/help.dat.
 * Does nothing if file not found.
 * Uses command file format with the following format:
 * - color: "red" "green" "blue" "save_slot"
 *   * sets text color to rgb and saves it to the selected reference slot
 * - color_ref: "save_slot"
 *   * sets text color from the selected reference slot
 * - margin: "pixels"
 *   * sets line margin
 * - sprite: "id" "x" "y" "anim_offset_x" "anim_offset_y"
 *   * shows sprite at x, y
 * - rect: "x" "y" "width" "height", "red", "green", "blue"
 *   * draws filled rectangle of selected size and color at x, y (upper left corner)
 * - page_end
 *   * waits for user input before proceeding
 * - doc_end
 *   * same as page_end but exits after it
 * - to_line: "line"
 *   * moves cursor to the start of the selected line
 * - default handler:
 *   * prints the text as is (escaping special characters such as ':' required)
 *   * special value "(empty line)" used for empty lines as command file format automatically
 *     skips empty lines. Also a whitespace only line could be used but it's not very obvious.
 */
void show_help(ALLEGRO_BITMAP *menu_sprites);