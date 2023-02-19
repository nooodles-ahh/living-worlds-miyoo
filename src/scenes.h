#ifndef SCENES_H
#define SCENES_H

#include <SDL/SDL.h>
#include "palette.h"

typedef struct Color_Remap_s
{
	SDL_Color col;
	int idx;
} Color_Remap_t;

typedef struct Scene_Info_s
{
	const char *filename;
	const char *title;
	int month_idx;
	const char *sound;
	float volume;
	unsigned int col_remapping_count;
	Color_Remap_t *col_remapping;
	LBM_Bitmap_t *loaded_bitmap;
} Scene_Info_t;

typedef struct Scenes_List_s
{
	unsigned int scene_count;
	Scene_Info_t *scenes;
} Scenes_List_t;

Scenes_List_t *scene_load_info_list(const char *file, char is_cycle);
LBM_Bitmap_t *scene_load_bitmap(Scenes_List_t *list, int idx);
#endif