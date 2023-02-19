#ifndef PALETTE_H
#define PALETTE_H
#include <SDL/SDL.h>
typedef struct Palette_cycle_s
{
	int reverse;
	int rate;
	int low;
	int high;
} Palette_cycle_t;

typedef struct Palette_s
{
	char *name;
	unsigned int width;
	unsigned int height;
	uint8_t *pixels;
	int cycle_count;
	Palette_cycle_t *cycles;
	unsigned int colors_count;
	SDL_Color *colors;
} Palette_t;

typedef struct Timeline_Entry_s
{
	unsigned int time;
	const char *name;
	Palette_t *palette;
	void *next;
	void *prev;
} Timeline_Entry_t;

typedef struct LBM_Bitmap_s
{
	Palette_t *base;
	unsigned int palettes_count;
	Palette_t **palettes;
	unsigned int timeline_count;
	Timeline_Entry_t **timeline;
} LBM_Bitmap_t;



#include "palette.h"
#include "loader.h"

#define PRECISION 100
#define DFLOAT_MOD(a, b) (double)((long)(a * PRECISION) % (b * PRECISION)) / PRECISION
#define CYCLE_SPEED 280

SDL_Color palette_fade_color(SDL_Color sourceColor, SDL_Color destColor, long frame, long max);
void palette_reverse_colors(SDL_Color *colors, Palette_cycle_t *cycle);
SDL_Color *parse_json_to_color_array(json_array_t *arr);
Palette_cycle_t *parse_json_to_cycles_array(json_array_t *arr);
Palette_t *parse_json_to_palette(json_value_t *val, char skip_pixels);
LBM_Bitmap_t *load_LBM_bitmap(const char *path);
void free_Palette(Palette_t *palette);
void free_LBM_bitmap(LBM_Bitmap_t *lbm);
void update_surface_cycle(SDL_Surface *surface, Uint64 time, unsigned int cycle_count, Palette_cycle_t *cycles, char blend);
#endif