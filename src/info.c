#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "info.h"

#define FADE_OUT_TIME 3000
#define FADE_OUT_TIME_END 5000

unsigned int last_ticks = 0;
int current_time = 0;
char palette_blending = 0;
const char *title = NULL;

TTF_Font *title_font = NULL;

void info_setup()
{
	title_font = TTF_OpenFont("HelveticaNeueBold.ttf", 18);
}

void info_set_title(const char *c)
{
	title = c;
}

SDL_Surface *info_create_surface(SDL_Surface *surface, unsigned int ticks, Scene_Info_t *scene, int time, char blend)
{
	#define TEXT_OFFSET_X 10
	#define TEXT_OFFSET_Y 25
	#define HOUR_TIME (60*60)
	char string[64];
	SDL_Color col = {255, 255, 255};
	SDL_Rect string_dest_rect = {TEXT_OFFSET_X, 480-TEXT_OFFSET_Y, 640, 480};

	// draw title
	snprintf(string, 64, "%s", scene->title);
	SDL_Surface *text = TTF_RenderUTF8_Blended(title_font, string, col);
	SDL_SetAlpha(text, SDL_SRCALPHA, 64);
	SDL_BlitSurface(text, NULL, surface, &string_dest_rect);
	SDL_FreeSurface(text);

	// draw time
	string_dest_rect.x = 640 - 60;
	snprintf(string, 64, "%02d:%02d", time/HOUR_TIME, (time/60)%60);
	text = TTF_RenderUTF8_Blended(title_font, string, col);
	SDL_SetAlpha(text, SDL_SRCALPHA, 64);
	SDL_BlitSurface(text, NULL, surface, &string_dest_rect);
	SDL_FreeSurface(text);


	return surface;
}