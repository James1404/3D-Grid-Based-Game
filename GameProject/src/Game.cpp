#include "Game.h"
#include "Entity.h"
#include "Entities/Player.h"
#include <iostream>


SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;
double Game::deltaTime;

Player* player;

Game::Game() { }
Game::~Game() { }

void Game::init(const char* title, int width, int height, bool fullscreen) {
	int flags = 0;
	if (fullscreen) {
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
		renderer = SDL_CreateRenderer(window, - 1, 0);
		if (renderer) {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		}

		isRunning = true;
	}
	player = new Player();
}

void Game::handleEvents() {
	SDL_PollEvent(&event);
	switch (event.type) {
	case SDL_QUIT:
		isRunning = false;
		break;
	default:
		break;
	}
}

Uint64 NOW = SDL_GetPerformanceCounter();
Uint64 LAST = 0;
void Game::update() {
	// calculate delta time;
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();

	deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());
	std::cout << deltaTime << std::endl;

	// update entities
	player->update();
}

void Game::render() {
	SDL_RenderClear(renderer);
	// draw stuff here
	player->render();
	SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
	SDL_RenderPresent(renderer);
}

void Game::clean() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}