#include "control.h"
#include "text.c"
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"
#include "SDL2/SDL_render.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#define MAX_ASTEROIDS 10
#define MIN_VERTICES 5
#define MAX_VERTICES 12
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256
#define SHIP_RADIUS 8
#define SCREEN_BORDER SHIP_RADIUS
#define BASE_SPEED 0.53*2
#define BASE_RADIUS 8
#define MAX_VEL BASE_SPEED
#define ACCEL MAX_VEL/60
#define ROT_SPEED 2*M_PI/60

float x;
float y;
float vx;
float vy;
float rot;

//bullet
float bx;
float by;
float bvx;
float bvy;
bool bullet_exists = false;

int score;
char score_string[1000];

struct Point {
	float x;
	float y;
};

struct Asteroid {
	bool exists;
	float x;
	float y;
	float vx;
	float vy;
	int num_vertices;
	struct Point vertices[MAX_VERTICES];
	int class;
};

SDL_Window* window;
SDL_Renderer* renderer;

struct Asteroid asteroids [MAX_ASTEROIDS] = {};

void
init() {
	//Check SDL Version
	SDL_version compiled;
	SDL_version linked;
	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	if (compiled.major != linked.major) {
		fprintf(stderr, "SDL version mismatch! Found version %d, require version %d", linked.major, compiled.major);
		exit(EXIT_FAILURE);
	}


	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER)) {
		fprintf(stderr, "Initialisation Error: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	//Hints
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	//Enable controllers
	SDL_GameControllerEventState(SDL_ENABLE);
	SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");

}

bool
point_in_poly(float x, float y, int num_vert, struct Point *verts) {
	bool inside = false;
	for(int i = 0, j = num_vert - 1; i < num_vert; j = i++) {
		if((y < verts[i].y && y > verts[j].y) || (y > verts[i].y && y < verts[j].y)) {
			if(x < (verts[j].x - verts[i].x) * (y - verts[i].y) / (verts[j].y - verts[i].y) + verts[i].x) {
				inside = !inside;
			}
		}
	}
	return inside;
}

bool
poly_in_poly(int x1, int y1, int count1, struct Point *poly1, int x2, int y2, int count2, struct Point *poly2) {
	for(int i = 0; i < count1; i++) {
		if(point_in_poly(poly1[i].x + x1 - x2, poly1[i].y + y1 - y2, count2, poly2)) {
			return true;
		}
	}
	return false;
}

void
draw_poly_offset(int x, int y, int num_vertices, struct Point *vertices) {
	for(int i = 0; i < num_vertices - 1; i++) {
		SDL_RenderDrawLine(renderer, x + vertices[i].x, y + vertices[i].y, x + vertices[i+1].x, y + vertices[i+1].y);
	}
	SDL_RenderDrawLine(renderer, x + vertices[num_vertices - 1].x, y + vertices[num_vertices - 1].y, x + vertices[0].x, y + vertices[0].y);
}

void
draw_asteroid(struct Asteroid a) {
	draw_poly_offset(a.x, a.y, a.num_vertices, a.vertices);
}

void
construct_ship_poly(struct Point verts[static 3]) {
	verts[0] = (struct Point){SHIP_RADIUS*cos(rot), SHIP_RADIUS*sin(rot)};
	verts[1] = (struct Point){SHIP_RADIUS*cos(rot - 3*M_PI/4), SHIP_RADIUS*sin(rot - 3*M_PI/4)};
	verts[2] = (struct Point){SHIP_RADIUS*cos(rot + 3*M_PI/4), SHIP_RADIUS*sin(rot + 3*M_PI/4)};
}

struct Asteroid
gen_asteroid(int x, int y, int class) {
	struct Asteroid a;
	a.x = x;
	a.y = y;
	float v = BASE_SPEED/class*(0.5 + (float)rand()/RAND_MAX);
	float dir = (float)rand()/RAND_MAX * 2*M_PI;
	a.vx = v*cos(dir);
	a.vy = v*sin(dir);
	int vertices = MIN_VERTICES + rand() % (MAX_VERTICES - MIN_VERTICES);
	float lengths[vertices];
	float angles[vertices];
	for(int i = 0; i < vertices; i++) {
		lengths[i] = BASE_RADIUS*class*(0.8 + (float)rand()/RAND_MAX*0.4);
		angles[i] = (i*2+1)*M_PI/(vertices);
		a.vertices[i].x = (int)lengths[i]*cos(angles[i]);
		a.vertices[i].y = (int)lengths[i]*sin(angles[i]);
	}
	a.num_vertices = vertices;
	a.class = class;
	a.exists = true;
	return a;
}

void
add_asteroid(struct Asteroid a) {
	for(int i = 0; i < MAX_ASTEROIDS; i++) {
		if(!asteroids[i].exists) {
			asteroids[i] = a;
			return;
		}
	}
}

void
destroy_asteroid(struct Asteroid *a) {
	int class = a->class;
	int x = a->x;
	int y = a->y;
	a->exists = false;
	//This is only notequals instead of less than equals because negative asteroids amuse me.
	if(class != 1) {
		add_asteroid(gen_asteroid(x, y, class - 1));
		add_asteroid(gen_asteroid(x, y, class - 1));
	}
	score += 100;
}

void
reset_player(){
	x = SCREEN_WIDTH/2;
	y = 3*SCREEN_HEIGHT/4;
	vx = 0;
	vy = 0;
	rot = -M_PI/2;
}

void
wait_for_key(SDL_Keycode sym) {
	bool wait = true;
	SDL_Event event;
	while(wait) {
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == sym) {
					wait = false;
				}
			} else if(event.type == SDL_QUIT) {
				SDL_Quit();
				exit(EXIT_SUCCESS);
			}
		}
		SDL_Delay(16);
	}
}

int
main(int argc, char **argv) {
	init();
	srand(getpid());

	if(SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_BORDERLESS, &window, &renderer)) {
		fprintf(stderr, "Window creation failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_SetWindowTitle(window, "Kuiper");

	//Text
	if(!load_fonts(renderer)) {
		printf("Error! Failed to load fonts: %s", IMG_GetError());
	}

	//Splash Screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	draw_string(renderer, "KUIPER!", SCREEN_WIDTH/2, SCREEN_HEIGHT/3, medium, center, middle);
	draw_string(renderer, "Press [space] to Start!", SCREEN_WIDTH/2, 2*SCREEN_HEIGHT/3, small, center, middle);
	SDL_Delay(100);
	SDL_RenderPresent(renderer);
	wait_for_key(SDLK_SPACE);

	while(1) {
		//set up for game start
		bool game_on = true;
		asteroids[0] = gen_asteroid(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 5);
		for(int i = 1; i < MAX_ASTEROIDS; i++) {
			asteroids[i].exists = false;
		}
		reset_player();
		score = 0;

		//game loop
		int t = SDL_GetTicks();
		int dt = 0;
		SDL_Event event;
		while(game_on) {
			//Begin Frame
			dt = SDL_GetTicks() - t;
			t += dt;
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			//Draw Score
			sprintf(score_string, "%d", score);
			draw_string(renderer, score_string, 0, 0, small, left, bottom);

			//EVENT LOOP
			while(SDL_PollEvent(&event)) {
				switch(event.type) {
					case SDL_KEYDOWN:
						switch(event.key.keysym.sym) {
							case SDLK_RIGHT:
								controller_state.right = true;
								break;
							case SDLK_LEFT:
								controller_state.left = true;
								break;
							case SDLK_UP:
								controller_state.up = true;
								break;
							case SDLK_DOWN:
								controller_state.down = true;
								break;
							case SDLK_SPACE:
								controller_state.shoot = true;
								if(!bullet_exists && !event.key.repeat) {
									bx = x + SHIP_RADIUS*cos(rot);
									by = y + SHIP_RADIUS*sin(rot);
									bvx = vx + 2*BASE_SPEED*cos(rot);
									bvy = vy + 2*BASE_SPEED*sin(rot);
									bullet_exists = true;
								}
								break;

						}
						break;
					case SDL_KEYUP:
						switch(event.key.keysym.sym) {
							case SDLK_RIGHT:
								controller_state.right = false;
								break;
							case SDLK_LEFT:
								controller_state.left = false;
								break;
							case SDLK_UP:
								controller_state.up = false;
								break;
							case SDLK_DOWN:
								controller_state.down = false;
								break;
							case SDLK_SPACE:
								controller_state.shoot = false;
								break;

						}
						break;
						//System Events
					case SDL_QUIT:
						SDL_Quit();
						exit(EXIT_SUCCESS);
				}
			}
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

			//player physics
			x += vx;
			y += vy;
			if(controller_state.up) {
				vx += ACCEL*cos(rot);
				vy += ACCEL*sin(rot);
				float magvel = sqrt(vx*vx + vy*vy);
				if (magvel > MAX_VEL) {
					vx = vx/magvel*MAX_VEL;
					vy = vy/magvel*MAX_VEL;
				}
			}
			if(controller_state.right && !controller_state.left) {
				rot += ROT_SPEED;
			} else if (controller_state.left && !controller_state.right) {
				rot -= ROT_SPEED;
			}
			//player wrap
			if(x > SCREEN_WIDTH + SCREEN_BORDER) {
				x -= SCREEN_WIDTH + 2*SCREEN_BORDER;
			} else if(x < -SCREEN_BORDER) {
				x += SCREEN_WIDTH + 2*SCREEN_BORDER;
			}
			if(y > SCREEN_HEIGHT + SCREEN_BORDER) {
				y -= SCREEN_HEIGHT + 2*SCREEN_BORDER;
			} else if(y < -SCREEN_BORDER) {
				y += SCREEN_HEIGHT + 2*SCREEN_BORDER;
			}
			struct Point ship_poly[3];
			construct_ship_poly(ship_poly);
			draw_poly_offset(x, y, 3, ship_poly);

			//bullet
			if(bullet_exists){
				bx += bvx;
				by += bvy;

				if(bx > SCREEN_WIDTH || bx < 0 || by > SCREEN_HEIGHT || by < 0) {
					bullet_exists = false;
				}
				SDL_RenderDrawPoint(renderer, bx, by);
			}

			//Asteroids
			bool asteroids_clear = true;
			for(int i = 0; i < MAX_ASTEROIDS; i++) {
				//Asteroid Physics
				if(!asteroids[i].exists) {
					continue;
				} else {
					asteroids_clear = false;
				}
				asteroids[i].x += asteroids[i].vx;
				asteroids[i].y += asteroids[i].vy;
				if(bullet_exists && point_in_poly(bx - asteroids[i].x, by - asteroids[i].y, asteroids[i].num_vertices, asteroids[i].vertices)) {
					destroy_asteroid(&asteroids[i]);
					bullet_exists = false;
				}
				//Wrap Asteroids
				float buffer_zone = 2*BASE_RADIUS*1.2*asteroids[i].class;
				if(asteroids[i].x > SCREEN_WIDTH + buffer_zone) {
					asteroids[i].x -= SCREEN_WIDTH + 2*buffer_zone;
				} else if(asteroids[i].x < -buffer_zone) {
					asteroids[i].x += SCREEN_WIDTH + 2*buffer_zone;
				}
				if(asteroids[i].y > SCREEN_HEIGHT + buffer_zone) {
					asteroids[i].y -= SCREEN_HEIGHT + 2*buffer_zone;
				} else if(asteroids[i].y < -buffer_zone) {
					asteroids[i].y += SCREEN_HEIGHT + 2*buffer_zone;
				}
				draw_asteroid(asteroids[i]);

				//Check for Player-Asteroid collision
				if(asteroids[i].exists && poly_in_poly(x, y, 3, ship_poly, asteroids[i].x, asteroids[i].y, asteroids[i].num_vertices, asteroids[i].vertices)) {
					reset_player();
					destroy_asteroid(&asteroids[i]);
					score -= 1000;
					if(score < 0) {
						score = 0;
					}
				}
			}

			if(asteroids_clear) {
				game_on = false;
			}

			//End Frame
			SDL_RenderPresent(renderer);
			dt = SDL_GetTicks() - t;
			if(dt < 16) {
				SDL_Delay(16 - dt);
			}
		}
		//Game Ended
		draw_string(renderer, "Your Score:", SCREEN_WIDTH/2, SCREEN_HEIGHT/3, small, center, middle);
		draw_string(renderer, score_string, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, medium, center, middle);
		draw_string(renderer, "Press 'R' to Restart", SCREEN_WIDTH/2, 2*SCREEN_HEIGHT/3, small, center, middle);
		SDL_RenderPresent(renderer);
		wait_for_key(SDLK_r);
	}
	return 0;
}
