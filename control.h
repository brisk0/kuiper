#ifndef CONTROL_H
#define CONTROL_H
#include <stdbool.h>
//Provides objects for game control outside the game context

struct ControllerState {
	bool shoot	:1;
	bool left	:1;
	bool right	:1;
	bool up		:1;
	bool down	:1;
} controller_state;

#endif
