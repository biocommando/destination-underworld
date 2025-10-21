
#include "help.h"
#include "sprites.h"
#include "duColors.h"
#include "duConstants.h"
#include "settings.h"
#include "command_file/generated/dispatch_help.h"

void dispatch__handle_help_color(struct help_color_DispatchDto *dto)
{
    dto->state->color = al_map_rgb(dto->red, dto->green, dto->blue);
    if (dto->ref >= 0)
    {
        dto->state->saved_colors[dto->ref] = dto->state->color;
    }
}

void dispatch__handle_help_color_ref(struct help_color_ref_DispatchDto *dto)
{
    dto->state->color = dto->state->saved_colors[dto->ref];
}

void dispatch__handle_help_margin(struct help_margin_DispatchDto *dto)
{
    dto->state->margin = dto->margin;
}

void dispatch__handle_help_sprite(struct help_sprite_DispatchDto *dto)
{
    draw_sprite_animated(dto->state->menu_sprites, dto->id, dto->x, dto->y,
        dto->anim_offset_x, dto->anim_offset_y);
}

void dispatch__handle_help_rect(struct help_rect_DispatchDto *dto)
{
    al_draw_filled_rectangle(dto->x, dto->y,
        dto->x + dto->width + 1, dto->y + dto->height + 1,
        al_map_rgb(dto->red, dto->green, dto->blue));
}

static int _help_handle_end(int doc_end)
{
    al_flip_display();
    int keys[] = {ALLEGRO_KEY_SPACE, ALLEGRO_KEY_ESCAPE};
    int key = wait_key_presses(keys, 2);
    al_clear_to_color(BLACK);
    if (doc_end || key == ALLEGRO_KEY_ESCAPE)
        return 1;
    return 0;
}

void dispatch__handle_help_page_end(struct help_page_end_DispatchDto *dto)
{
    dto->state->line = 0;
    if (_help_handle_end(0) != 0)
        strcpy(dto->skip_label, "to end");
}

void dispatch__handle_help_doc_end(struct help_doc_end_DispatchDto *dto)
{
    dto->state->line = 0;
    if (_help_handle_end(1) != 0)
        strcpy(dto->skip_label, "to end");
}

void dispatch__handle_help_to_line(struct help_to_line_DispatchDto *dto)
{
    dto->state->line = dto->line;
}

void dispatch__handle_help_default(struct help_default_DispatchDto *dto)
{
    const char *text = dto->command;
    HelpState *state = dto->state;
    if (!strcmp(text, "(empty line)"))
    {
        state->line++;
        return;
    }
    const int line_height = 16;
    const int y_margin = 5;
    al_draw_textf(get_font(), state->color,
        state->margin, y_margin + state->line * line_height, 0, text);
    state->line++;
}

void show_help(ALLEGRO_BITMAP *menu_sprites)
{
    char help_path[256];
    sprintf(help_path, DATADIR "%s/help.dat", get_game_settings()->mission_pack);

    HelpState state;
    memset(&state, 0, sizeof(state));
    state.menu_sprites = menu_sprites;
    state.color = al_map_rgb(255, 255, 255);
    state.margin = 5;
    al_clear_to_color(BLACK);

    read_command_file(help_path, dispatch__help, &state);
}