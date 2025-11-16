#include "sprites.h"

#include "logging.h"
#include "command_file/generated/dispatch_sprite_conf.h"

static DuSprite sprites[SPRITE_ID_MAX + 1];
static int sprite_ok[SPRITE_ID_MAX + 1];
static int init_ok = 0;

static inline int id_ok(int id)
{
    return id <= SPRITE_ID_MAX && id >= SPRITE_ID_MIN;
}

static inline int check_sprite_ok(int id)
{
    return init_ok && id_ok(id) && sprite_ok[id];
}

void init_sprites()
{
    if (!init_ok)
    {
        memset(sprites, 0, sizeof(sprites));
        memset(sprite_ok, 0, sizeof(sprite_ok));
        init_ok = 1;
    }
}

int set_sprite(int sprite_id, DuSprite sprite)
{
    if (!id_ok(sprite_id))
        return -1;
    init_sprites();
    sprites[sprite_id] = sprite;
    sprite_ok[sprite_id] = 1;
    return 0;
}

DuSprite *get_sprite(int sprite_id)
{
    if (!check_sprite_ok(sprite_id))
        return NULL;
    return &sprites[sprite_id];
}

void dispatch__handle_sprite_conf_sprite(struct sprite_conf_sprite_DispatchDto *dto)
{
    DuSprite *s = &dto->state[dto->id];
    s->sx = dto->x;
    s->sy = dto->y;
    s->width = dto->width;
    s->height = dto->height;
    sprite_ok[dto->id] = 1;
    LOG("Read sprite %d : %d %d %d %d\n", dto->id, s->sx, s->sy, s->width, s->height);
}

int read_sprites_from_file(const char *filename)
{
    init_sprites();
    read_command_file(filename, dispatch__sprite_conf, sprites);
    return 0;
}

void reset_sprites()
{
    init_ok = 0;
}

void draw_sprite(const ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y)
{
    DuSprite *s = get_sprite(sprite_id);
    if (s)
    {
        al_draw_bitmap_region((ALLEGRO_BITMAP*)sprite_sheet, s->sx, s->sy, s->width, s->height, x, y, 0);
    }
}

void draw_sprite_centered(const ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y)
{
    DuSprite *s = get_sprite(sprite_id);
    if (s)
    {
        al_draw_bitmap_region((ALLEGRO_BITMAP*)sprite_sheet, s->sx, s->sy, s->width, s->height, x - s->width / 2, y - s->height / 2, 0);
    }
}

void draw_sprite_animated(const ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y, int dx, int dy)
{
    DuSprite *s = get_sprite(sprite_id);
    if (s)
    {
        al_draw_bitmap_region((ALLEGRO_BITMAP*)sprite_sheet, s->sx + s->width * dx, s->sy + s->height * dy, s->width, s->height, x, y, 0);
    }
}

void draw_sprite_animated_centered(const ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y, int dx, int dy)
{
    DuSprite *s = get_sprite(sprite_id);
    if (s)
    {
        al_draw_bitmap_region((ALLEGRO_BITMAP*)sprite_sheet, s->sx + s->width * dx, s->sy + s->height * dy, s->width, s->height,
                              x - s->width / 2, y - s->height / 2, 0);
    }
}

void draw_sprite_scaled_animated_centered(const ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y, int dx, int dy, double scale)
{
    DuSprite *s = get_sprite(sprite_id);
    if (s)
    {
        double w = s->width * scale, h = s->height * scale;
        al_draw_scaled_bitmap((ALLEGRO_BITMAP*)sprite_sheet, s->sx + s->width * dx, s->sy + s->height * dy, s->width, s->height,
                              x - w / 2, y - h / 2, w, h, 0);
    }
}