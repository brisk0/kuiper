#include <SDL2/SDL_image.h>
#include <stdbool.h>

SDL_Texture * font_small;
SDL_Texture * font_medium;
SDL_Texture * font_large;

enum Align {
	left,
	right,
	center
};

enum AlignVert {
	top,
	bottom,
	middle
};

enum FontSize {
	small = 8,
	medium = 16,
	large = 32
};

//Must be used before text drawing functions or you will segfault probably.
bool
load_fonts(SDL_Renderer *renderer) {
	bool success = true;
	IMG_Init(IMG_INIT_PNG);
	if(!(font_small = IMG_LoadTexture(renderer, "font_small.png"))) {
		success = false;
	}
	if(!(font_medium = IMG_LoadTexture(renderer, "font_medium.png"))) {
		success = false;
	}
		IMG_Quit();
		return success;
}

void
draw_string(SDL_Renderer * renderer, char * string, int x, int y, enum FontSize font_size, enum Align align, enum AlignVert align_vert) {
	SDL_Texture *font;
	switch(font_size) {
		case small:
			font = font_small;
			break;
		case medium:
			font = font_medium;
			break;
		case large:
			font = font_large;
			break;
	}
	if(align != left) {
		int num_chars = 0;
		while(string[num_chars]) {
			num_chars++;
		}
		if(align == right) {
			x -= num_chars*font_size;
		} else if(align == center) {
			x -= num_chars*font_size/2;
		}
	}
	if(align_vert == top) {
		y -= font_size;
	} else if(align_vert == middle) {
		y -= font_size/2;
	}
	int i = 0;

	while(string[i]) {
		SDL_Rect src = (SDL_Rect){(string[i] - 32)*font_size, 0, font_size, font_size};
		SDL_Rect dest = (SDL_Rect){x + i*font_size, y, font_size, font_size};
		SDL_RenderCopy(renderer, font, &src, &dest);
		i++;
	}
}
