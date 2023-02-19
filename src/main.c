#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include "json.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "palette.h"
#include "scenes.h"
#include <assert.h>
#include <pthread.h>
#include "info.h"
#include "keymap_sw.h"

#define WIDTH 640
#define HEIGHT 480
#define FPS 1000 / 30
#define TIME_INC_HOUR 3600
#define TIME_INC_STEP 600

SDL_Surface *g_window_surface = NULL;

void change_image(SDL_Surface **surface, LBM_Bitmap_t **bitmap, Scenes_List_t *scenes_list, int idx, Mix_Music **music)
{
	if( *music )
		Mix_FreeMusic(*music);
	*music = NULL;

	*bitmap = scene_load_bitmap(scenes_list, idx);
	// Create our new surface and copy everything to it
	if (!(*surface) || (*surface)->w != (*bitmap)->base->width || (*surface)->h != (*bitmap)->base->height)
	{
		SDL_Surface *new_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
										(*bitmap)->base->width, (*bitmap)->base->height, 8, 0, 0, 0, 0);

		if (!surface)
		{
			*surface = new_surface;
		}
		else
		{
			SDL_Surface *old_surface = *surface;
			*surface = new_surface;
			SDL_FreeSurface(old_surface);
		}
	}
	SDL_SetPalette(*surface, SDL_LOGPAL, (*bitmap)->base->colors, 0, (*bitmap)->base->colors_count);
	memcpy((*surface)->pixels, (*bitmap)->base->pixels, (*bitmap)->base->width * (*bitmap)->base->height);

	if(scenes_list->scenes[idx].sound )
	{
		char path[64];
		snprintf(path, 64, "audio/%s.ogg", scenes_list->scenes[idx].sound);
		*music = Mix_LoadMUS(path);
		Mix_VolumeMusic(scenes_list->scenes[idx].volume * 40);
		Mix_PlayMusic(*music, -1);
	}
}

// this isn't safe from a race condition...
void *load_bitmaps(void *data)
{
	Scenes_List_t *list = (Scenes_List_t *)data;
	// memory usage is mimimal, load all of them now
	for (int i = 0; i < list->scene_count; ++i)
		scene_load_bitmap(list, i);

	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char *argv[])
{
	TTF_Init();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	g_window_surface = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_HWSURFACE | SDL_SRCALPHA | SDL_DOUBLEBUF);
	SDL_ShowCursor(SDL_DISABLE);
	Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16SYS, 2, 2048);
	Mix_Init(MIX_INIT_OGG | MIX_INIT_MP3);

	info_setup();

	Mix_Music *music = NULL;

	// read our scenes json
	Scenes_List_t *cycle_scenes_list = scene_load_info_list("cycle_scenes.json", 1);
	Scenes_List_t *world_scenes_list = scene_load_info_list("world_scenes.json", 0);

	// load the first bitmap for both niceness and ensuring that we actually have a bitmap
	Scenes_List_t *scenes_list = cycle_scenes_list;
	LBM_Bitmap_t *bitmap = scene_load_bitmap(scenes_list, 0);
	if (!bitmap)
	{
		return 1;
	}

	// 2 threads creating a race condition that I simply choose to ignore
	if(cycle_scenes_list)
	{
		pthread_t loading_thread;
		pthread_create(&loading_thread, NULL, load_bitmaps, (void *)cycle_scenes_list);
	}

	if(world_scenes_list)
	{
		pthread_t loading_thread2;
		pthread_create(&loading_thread2, NULL, load_bitmaps, (void *)world_scenes_list);
	}

	SDL_Surface *bitmap_surface = NULL;
	change_image(&bitmap_surface, &bitmap, scenes_list, 0, &music);

	int r_w = g_window_surface->w;
	int r_h = g_window_surface->h;
	SDL_Rect src_rect = {0, 0, bitmap->base->width, bitmap->base->height};
	SDL_Rect dest_rect = {0, 0, r_w, r_h};

	// current scene index
	int cyc_bitmap_idx, wld_bitmap_idx;
	cyc_bitmap_idx = wld_bitmap_idx = 0;
	int current_idx = cyc_bitmap_idx;

	char blend = 1;
	char quit = 0;
	char fill_mode = 0;
	char is_cycle = 1;
	unsigned int current_time = 32400; // 9am
	int blend_amount = 0;
	while (!quit)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e) > 0)
		{
			switch (e.type)
			{
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_KEYDOWN:
				switch (e.key.keysym.sym)
				{
				// B
				case SW_BTN_B:
					blend = !blend;
					break;

				// SELECT
				case SW_BTN_SELECT:
					// Lazily only change list type if we actually have that list
					if(world_scenes_list)
						is_cycle = !is_cycle;
					scenes_list = is_cycle ? cycle_scenes_list : world_scenes_list;

					if(is_cycle)
					{
						wld_bitmap_idx = current_idx;
						current_idx = cyc_bitmap_idx;
					}
					else
					{
						cyc_bitmap_idx = current_idx;
						current_idx = wld_bitmap_idx;
					}

					if (current_idx >= scenes_list->scene_count)
						current_idx = scenes_list->scene_count - 1;

					change_image(&bitmap_surface, &bitmap, scenes_list, current_idx, &music);

					break;
				case SW_BTN_START:
					quit = 1;
					break;

				case SW_BTN_UP:
					if (((int)current_time - TIME_INC_HOUR) < 0)
						current_time = (86400 - TIME_INC_HOUR) + current_time;
					else
						current_time -= TIME_INC_HOUR;
					break;

				case SW_BTN_DOWN:
					current_time += TIME_INC_HOUR;
					if (current_time >= 86400)
						current_time -= 86400;
					break;

				case SW_BTN_LEFT:
					if (((int)current_time - TIME_INC_STEP) < 0)
						current_time = (86400 - TIME_INC_STEP) + current_time;
					else
						current_time -= TIME_INC_STEP;
					break;

				case SW_BTN_RIGHT:
					current_time += TIME_INC_STEP;
					if (current_time >= 86400)
						current_time -= 86400;
					break;

				case SW_BTN_L1:
				case SW_BTN_L2:
					current_idx--;
					if (current_idx < 0)
						current_idx = scenes_list->scene_count - 1;
					change_image(&bitmap_surface, &bitmap, scenes_list, current_idx, &music);
					break;
				case SW_BTN_R1:
				case SW_BTN_R2:
					current_idx++;
					if (current_idx >= scenes_list->scene_count)
						current_idx = 0;
					change_image(&bitmap_surface, &bitmap, scenes_list, current_idx, &music);
					break;
				}
			}
		}

		// move cycle
		Palette_t *palette = bitmap->base;

		SDL_SetPalette(bitmap_surface, SDL_LOGPAL, palette->colors, 0, palette->colors_count);
		// do we have extra palettes? i.e. do we have a day-night cycle
		if (bitmap->palettes_count)
		{
			Timeline_Entry_t *time_entry = bitmap->timeline[0];
			for (int i = 0; i < bitmap->timeline_count; ++i)
			{
				Timeline_Entry_t *prev = bitmap->timeline[i]->prev;
				Timeline_Entry_t *next = bitmap->timeline[i]->next;
				Timeline_Entry_t *sel = bitmap->timeline[i];

				if(bitmap->timeline[i]->time >= current_time)
				{
					time_entry = bitmap->timeline[i];
					break;
				}
			}
			Timeline_Entry_t *next_time_entry = (Timeline_Entry_t *)(time_entry->next);

			blend_amount = (int)((next_time_entry->time - current_time) /(float)(next_time_entry->time - time_entry->time));
			for (int i = 0; i < time_entry->palette->colors_count; ++i)
			{
				bitmap_surface->format->palette->colors[i] = palette_fade_color(time_entry->palette->colors[i], next_time_entry->palette->colors[i], blend_amount, PRECISION);
			}
		}

		update_surface_cycle(bitmap_surface, SDL_GetTicks(), palette->cycle_count, palette->cycles, blend);
		SDL_Surface *optimised_surface = SDL_ConvertSurface(bitmap_surface, g_window_surface->format, 0);
		info_create_surface(optimised_surface, SDL_GetTicks(), &scenes_list->scenes[current_idx], current_time, blend);

		SDL_BlitSurface(optimised_surface, &src_rect, g_window_surface, &dest_rect);
		SDL_FreeSurface(optimised_surface);

		SDL_Flip(g_window_surface);

		SDL_Delay(FPS);
	}

	free_LBM_bitmap(bitmap);
	SDL_FreeSurface(bitmap_surface);
	Mix_FreeMusic(music);
	Mix_CloseAudio();
	SDL_Quit();

	return 0;
}