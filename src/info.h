#ifndef INFO_H
#define INFO_H
#include <SDL/SDL.h>
#include "scenes.h"

void info_setup();
SDL_Surface *info_create_surface(SDL_Surface *surface, unsigned int ticks, Scene_Info_t *scene, int time, char blend);

#endif