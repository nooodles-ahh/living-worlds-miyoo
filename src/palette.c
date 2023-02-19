// Much of this code is based on https://github.com/jhuckaby/canvascycle
#include "palette.h"
#include "loader.h"
#include <math.h>

#define PRECISION 100
#define DFLOAT_MOD(a, b) (double)((long)(a * PRECISION) % (b * PRECISION)) / PRECISION
#define CYCLE_SPEED 280

SDL_Color palette_fade_color(SDL_Color sourceColor, SDL_Color destColor, long frame, long max)
{
	SDL_Color tempColor;

	if (!max)
		return sourceColor; // avoid divide by zero
	if (frame < 0)
		frame = 0;
	if (frame > max)
		frame = max;

	tempColor.r = sourceColor.r + (((long)(destColor.r - sourceColor.r) * frame) / max);
	tempColor.g = sourceColor.g + (((long)(destColor.g - sourceColor.g) * frame) / max);
	tempColor.b = sourceColor.b + (((long)(destColor.b - sourceColor.b) * frame) / max);

	return (tempColor);
}

void palette_reverse_colors(SDL_Color *colors, Palette_cycle_t *cycle)
{
	short i;
	short cycleSize = (cycle->high - cycle->low) + 1;

	SDL_Color temp;
	for (i = 0; i < cycleSize / 2; i++)
	{
		temp = colors[cycle->low + i];
		colors[cycle->low + i] = colors[cycle->high - i];
		colors[cycle->high - i] = temp;
	}
}

SDL_Color *parse_json_to_color_array(json_array_t *arr)
{
	SDL_Color *colors = (SDL_Color *)malloc(sizeof(SDL_Color) * arr->length);
	json_array_element_t *elm = arr->start;
	for (int i = 0; i < arr->length; ++i)
	{
		json_array_t *rgb_arr = json_value_as_array(elm->value);
		json_array_element_t *rgb_elm = rgb_arr->start;
		// r
		colors[i].r = atoi(json_value_as_number(rgb_elm->value)->number);
		rgb_elm = rgb_elm->next;
		//// g
		colors[i].g = atoi(json_value_as_number(rgb_elm->value)->number);
		rgb_elm = rgb_elm->next;
		// b
		colors[i].b = atoi(json_value_as_number(rgb_elm->value)->number);

		elm = elm->next;
	}

	return colors;
}

Palette_cycle_t *parse_json_to_cycles_array(json_array_t *arr)
{
	Palette_cycle_t *cycles = (Palette_cycle_t *)malloc(sizeof(Palette_cycle_t) * arr->length);
	json_array_element_t *elm = arr->start;
	for (int i = 0; i < arr->length; ++i)
	{
		json_object_t *cycle_obj = json_value_as_object(elm->value);
		json_object_element_t *cycle_obj_elm = cycle_obj->start;
		// would be nice if could just assume the ordering, but I'm not gonna
		while (cycle_obj_elm)
		{
			if (!strcmp(cycle_obj_elm->name->string, "reverse"))
			{
				cycles[i].reverse = SDL_atoi(json_value_as_number(cycle_obj_elm->value)->number);
			}
			else if (!strcmp(cycle_obj_elm->name->string, "rate"))
			{
				cycles[i].rate = SDL_atoi(json_value_as_number(cycle_obj_elm->value)->number);
			}
			else if (!strcmp(cycle_obj_elm->name->string, "high"))
			{
				cycles[i].high = SDL_atoi(json_value_as_number(cycle_obj_elm->value)->number);
			}
			else if (!strcmp(cycle_obj_elm->name->string, "low"))
			{
				cycles[i].low = SDL_atoi(json_value_as_number(cycle_obj_elm->value)->number);
			}

			cycle_obj_elm = cycle_obj_elm->next;
		}

		elm = elm->next;
	}
	return cycles;
}

Palette_t *parse_json_to_palette(json_value_t *val, char skip_pixels)
{
	if (!val)
		return NULL;

	Palette_t *new_palette = (Palette_t *)malloc(sizeof(Palette_t));

	// zero out everything just incase
	memset(new_palette, 0, sizeof(Palette_t));

	json_object_t *obj = json_value_as_object(val);
	json_object_element_t *elm = obj->start;
	while (elm)
	{
		if (!strcmp(elm->name->string, "width"))
		{
			new_palette->width = SDL_atoi(json_value_as_number(elm->value)->number);
		}
		else if (!strcmp(elm->name->string, "height"))
		{
			new_palette->height = SDL_atoi(json_value_as_number(elm->value)->number);
		}
		else if (!strcmp(elm->name->string, "colors"))
		{
			json_array_t *color_array = json_value_as_array(elm->value);
			new_palette->colors = parse_json_to_color_array(color_array);
			new_palette->colors_count = color_array->length;
		}
		else if (!strcmp(elm->name->string, "cycles"))
		{
			json_array_t *cycles_array = json_value_as_array(elm->value);
			new_palette->cycles = parse_json_to_cycles_array(cycles_array);
			new_palette->cycle_count = cycles_array->length;
		}
		else if (!strcmp(elm->name->string, "pixels") && !skip_pixels)
		{
			// lol just do it here
			json_array_t *pixels_arr = json_value_as_array(elm->value);
			json_array_element_t *pixels_arr_elm = pixels_arr->start;
			new_palette->pixels = (unsigned char *)malloc(sizeof(unsigned char) * pixels_arr->length);
			for (int i = 0; i < pixels_arr->length; ++i)
			{
				new_palette->pixels[i] = SDL_atoi(
					json_value_as_number(pixels_arr_elm->value)->number);

				pixels_arr_elm = pixels_arr_elm->next;
			}
		}

		elm = elm->next;
	}

	return new_palette;
}

LBM_Bitmap_t *load_LBM_bitmap(const char *path)
{
	json_value_t *loaded_json = read_json(path);
	if (!loaded_json)
		return NULL;

	LBM_Bitmap_t *new_bitmap = (LBM_Bitmap_t *)malloc(sizeof(LBM_Bitmap_t));
	memset(new_bitmap, 0, sizeof(LBM_Bitmap_t));
	json_object_t *json_obj = json_value_as_object(loaded_json);
	json_object_element_t *json_obj_elm = json_obj->start;
	while (json_obj_elm)
	{
		if (!strcmp(json_obj_elm->name->string, "base"))
		{
			new_bitmap->base = parse_json_to_palette(json_obj_elm->value, 0);
		}
		else if (!strcmp(json_obj_elm->name->string, "palettes"))
		{
			json_object_t *pals_obj = json_value_as_object(json_obj_elm->value);
			json_object_element_t *pals_elm = pals_obj->start;
			new_bitmap->palettes_count = pals_obj->length;
			new_bitmap->palettes = (Palette_t **)malloc(sizeof(Palette_t *) * pals_obj->length);

			for (int i = 0; i < pals_obj->length; ++i)
			{
				new_bitmap->palettes[i] = parse_json_to_palette(pals_elm->value, 1);
				new_bitmap->palettes[i]->name = SDL_strdup(pals_elm->name->string);
				pals_elm = pals_elm->next;
			}
		}
		else if(!strcmp(json_obj_elm->name->string, "timeline"))
		{
			json_object_t *timeline_obj = json_value_as_object(json_obj_elm->value);
			json_object_element_t *timeline_elm = timeline_obj->start;
			new_bitmap->timeline_count = timeline_obj->length;
			new_bitmap->timeline = (Timeline_Entry_t **)malloc(sizeof(Timeline_Entry_t *) * timeline_obj->length);

			for (int i = 0; i < timeline_obj->length; ++i)
			{
				new_bitmap->timeline[i] = (Timeline_Entry_t *)malloc(sizeof(Timeline_Entry_t));
				new_bitmap->timeline[i]->name = SDL_strdup(json_value_as_string(timeline_elm->value)->string);
				new_bitmap->timeline[i]->time = SDL_atoi(timeline_elm->name->string);
				new_bitmap->timeline[i]->palette = NULL;
				timeline_elm = timeline_elm->next;
			}

		}
		// just a single image, no additional palettes
		else if (!strcmp(json_obj_elm->name->string, "filename"))
		{
			new_bitmap->base = parse_json_to_palette(loaded_json, 0);
			break;
		}

		json_obj_elm = json_obj_elm->next;
	}

	// map palettes to timeline
	Timeline_Entry_t **time_entry_list = new_bitmap->timeline;
	for(int i = 0; i < new_bitmap->timeline_count; ++i)
	{
		time_entry_list[i]->prev = time_entry_list[(i - 1) < 0 ?  (new_bitmap->timeline_count-1) : (i -1) ];
		time_entry_list[i]->next = time_entry_list[(i + 1) % new_bitmap->timeline_count];
		for(int j = 0; j < new_bitmap->palettes_count; ++j)
		{
			if(!strcasecmp(time_entry_list[i]->name, new_bitmap->palettes[j]->name))
			{
				time_entry_list[i]->palette = new_bitmap->palettes[j];
				break;
			}
		}
	}

	free(loaded_json);

	return new_bitmap;
}

void free_Palette(Palette_t *palette)
{
	free(palette->name);
	free(palette->colors);
	free(palette->cycles);
	free(palette->pixels);
	free(palette);
}

void free_LBM_bitmap(LBM_Bitmap_t *lbm)
{
	if(!lbm)
		return;
		
	free(lbm->timeline);
	for (int i = 0; i < lbm->palettes_count; ++i)
	{
		free_Palette(lbm->palettes[i]);
	}
	free(lbm->palettes);
	free_Palette(lbm->base);
	free(lbm);
}

void update_surface_cycle(SDL_Surface *surface, Uint64 time, unsigned int cycle_count, Palette_cycle_t *cycles, char blend)
{
	for (int i = 0; i < cycle_count; ++i)
	{
		Palette_cycle_t *cycle = &cycles[i];
		if (cycle->rate == 0)
			continue;

		int cycleSize = (cycle->high - cycle->low) + 1;
		int cycleRate = cycle->rate / CYCLE_SPEED;

		double cycleAmount = 0.0;

		if (cycles[i].reverse < 3)
			cycleAmount = DFLOAT_MOD((double)(time / (1000 / (double)cycleRate)), cycleSize);
		else if (cycles[i].reverse == 3)
		{
			cycleAmount = DFLOAT_MOD((double)(time / (1000 / (double)cycleRate)), cycleSize * 2);
			if (cycleAmount >= cycleSize)
				cycleAmount = (cycleSize * 2) - cycleAmount;
		}
		else if (cycles[i].reverse < 6)
		{
			cycleAmount = DFLOAT_MOD((double)(time / (1000 / (double)cycleRate)), cycleSize);
			cycleAmount = sin((cycleAmount * 3.1415926 * 2) / cycleSize) + 1;
			if (cycles[i].reverse == 4)
				cycleAmount *= (cycleSize / 4);
			else if (cycles[i].reverse == 5)
				cycleAmount *= (cycleSize / 2);
		}

		SDL_Color *colors = surface->format->palette->colors;

		if (cycles[i].reverse == 2)
			palette_reverse_colors(colors, &cycles[i]);

		for (int i = 0; i < (int)cycleAmount; i++)
		{
			SDL_Color temp = colors[cycle->high];
			for (int j = cycle->high - 1; j >= cycle->low; j--)
			{
				colors[j + 1] = colors[j];
			}
			colors[cycle->low] = temp;
		}

		short frame = (short)((double)(cycleAmount - (short)cycleAmount) * PRECISION);

		if (blend)
		{
			SDL_Color temp = colors[cycle->high];
			for (int j = cycle->high - 1; j >= cycle->low; j--)
				colors[j + 1] = palette_fade_color(colors[j + 1], colors[j], frame, PRECISION);
			colors[cycle->low] = palette_fade_color(colors[cycle->low], temp, frame, PRECISION);
		}

		if (cycles[i].reverse == 2)
			palette_reverse_colors(colors, &cycles[i]);
	}
}