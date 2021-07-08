#include <SDL.h>
#include "Game.h"

Game game;

int main(int argc, char* args[]) {
	game.init("Window");

	while (game.running()) {
		game.handleEvents();
		game.update();
		game.render();
	}

	game.clean();

	return 0;
}