#include "sprites.h"

#include "record_file.h"
#include "logging.h"

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

int read_sprites_from_file(const char *filename, int min_id, int max_id)
{
    if (!id_ok(min_id) || !id_ok(max_id))
    {
        LOG("Invalid sprite id range %d...%d\n", min_id, max_id);
        return -1;
    }
    init_sprites();
    for (int i = SPRITE_ID_MIN; i <= SPRITE_ID_MAX; i++)
    {
        char key[] = "sprite_xxx";
        sprintf(key, "sprite_%d", i);
        DuSprite *s = &sprites[i];
        record_file_scanf(filename, key, "%*s x=%d y=%d w=%d h=%d", &s->sx, &s->sy, &s->width, &s->height);
        if (s->width != 0 && s->height != 0)
        {
            sprite_ok[i] = 1;
            LOG("Read sprite %d : %d %d %d %d\n", i, s->sx, s->sy, s->width, s->height);
        }
        else
        {
            sprite_ok[i] = 0;
            LOG("ERROR: invalid sprite configuration for sprite %d\n", i);
        }
    }
    return 0;
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