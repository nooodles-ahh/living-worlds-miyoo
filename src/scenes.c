#include "loader.h"
#include "palette.h"
#include "scenes.h"
#include <stdlib.h>
#include <string.h>

void scene_apply_color_remappings(Palette_t *pal, Color_Remap_t *remap, int count)
{
	for(int i = 0; i < count; ++i)
	{
		pal->colors[remap[i].idx] = remap[i].col;
	}
}

LBM_Bitmap_t *scene_load_bitmap(Scenes_List_t *list, int idx)
{
	if (!list)
		return NULL;

	if (idx > list->scene_count || idx < 0)
		return NULL;

	// have we already loaded this?
	if(list->scenes[idx].loaded_bitmap)
		return list->scenes[idx].loaded_bitmap;
	
	char filename[256];
	snprintf(filename, 256, "images/%s.json", list->scenes[idx].filename);
	LBM_Bitmap_t *new_bitmap = load_LBM_bitmap(filename);
	if (!new_bitmap)
		return NULL;

	// override any colours remappings
	scene_apply_color_remappings(new_bitmap->base,
								 list->scenes[idx].col_remapping,
								 list->scenes[idx].col_remapping_count);
	for (int i = 0; i < new_bitmap->palettes_count; ++i)
	{
		scene_apply_color_remappings(new_bitmap->palettes[i],
									 list->scenes[idx].col_remapping,
									 list->scenes[idx].col_remapping_count);
	}

	list->scenes[idx].loaded_bitmap = new_bitmap;

	return new_bitmap;
}


Scenes_List_t *scene_load_info_list(const char *file, char is_cycle)
{
	json_value_t *loaded_json = read_json(file);
	if (!loaded_json)
		return NULL;

	json_array_t *scenes_arr = json_value_as_array(loaded_json);
	json_array_element_t *elm = scenes_arr->start;

	Scenes_List_t *new_list = (Scenes_List_t *)malloc(sizeof(Scenes_List_t));
	Scene_Info_t *new_scenes = (Scene_Info_t *)malloc(sizeof(Scene_Info_t) * scenes_arr->length);

	new_list->scenes = new_scenes;
	new_list->scene_count = scenes_arr->length;

	memset(new_scenes, 0, sizeof(Scene_Info_t) * scenes_arr->length);
	for (int i = 0; i < new_list->scene_count; ++i)
	{
		json_object_t *scene_entry = json_value_as_object(elm->value);
		json_object_element_t *scene_elm = scene_entry->start;
		new_scenes[i].volume = 1.0f;
		while (scene_elm)
		{
			if (!strcmp("title", scene_elm->name->string))
			{
				new_scenes[i].title = SDL_strdup(json_value_as_string(scene_elm->value)->string);
			}
			else if (!strcmp("name", scene_elm->name->string))
			{
				if(is_cycle )
				{
					char *name = (char *)malloc(sizeof(char) * 128);
					snprintf(name, 128, "%s.LBM", json_value_as_string(scene_elm->value)->string);
					new_scenes[i].filename = name;
				}
				else
				{
					new_scenes[i].filename = SDL_strdup(json_value_as_string(scene_elm->value)->string);
				}
			}
			else if (!strcmp("monthIdx", scene_elm->name->string))
			{
				new_scenes[i].month_idx = SDL_atoi(json_value_as_number(scene_elm->value)->number);
			}
			else if (!strcmp("sound", scene_elm->name->string))
			{
				new_scenes[i].sound = SDL_strdup(json_value_as_string(scene_elm->value)->string);
			}
			else if (!strcmp("maxVolume", scene_elm->name->string))
			{
				new_scenes[i].volume = SDL_atof(json_value_as_number(scene_elm->value)->number);
			}
			else if (!strcmp("remap", scene_elm->name->string))
			{
				json_object_t *remap_obj = json_value_as_object(scene_elm->value);
				json_object_element_t *remap_elm = remap_obj->start;

				Color_Remap_t *remappings = (Color_Remap_t *)malloc(sizeof(Color_Remap_t) * remap_obj->length);
				for (int j = 0; j < remap_obj->length; ++j)
				{
					// the index
					remappings[j].idx = SDL_atoi(remap_elm->name->string);

					// colour
					json_array_t *rgb_arr = json_value_as_array(remap_elm->value);
					json_array_element_t *rgb_elm = rgb_arr->start;
					// r
					remappings[j].col.r = atoi(json_value_as_number(rgb_elm->value)->number);
					rgb_elm = rgb_elm->next;
					// g
					remappings[j].col.g = atoi(json_value_as_number(rgb_elm->value)->number);
					rgb_elm = rgb_elm->next;
					// b
					remappings[j].col.b = atoi(json_value_as_number(rgb_elm->value)->number);

					remap_elm = remap_elm->next;
				}

				new_scenes[i].col_remapping_count = remap_obj->length;
				new_scenes[i].col_remapping = remappings;
			}
			scene_elm = scene_elm->next;
		}
		elm = elm->next;
	}
	free(loaded_json);
	return new_list;
}