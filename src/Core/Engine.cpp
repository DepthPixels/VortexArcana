#include "Engine.h"
#include <iostream>

Engine::Engine()
	: m_window(nullptr), m_renderer(nullptr), m_isRunning(false) {
	// Constructor Defaults.
}

Engine::~Engine() {
	// Destructor to free resources and ensure Shutdown is called.
	Shutdown();
}

bool Engine::Initialize() {
	// Initialize SDL Audio and Video.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == false) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}

	// Create the SDL Window.
	m_window = SDL_CreateWindow("VortexArcana Engine v0.0.1", 1280, 720, SDL_WINDOW_RESIZABLE);
	if (!m_window) {
		std::cerr << "Window Creation Error: " << SDL_GetError() << std::endl;
		return false;
	}

	// Putting in NULL as name of renderer is basically auto.
	m_renderer = SDL_CreateRenderer(m_window, NULL);
	if (!m_renderer) {
		std::cerr << "Renderer Creation Error: " << SDL_GetError() << std::endl;
		return false;
	}

	// If nothing failed yet then initialization worked!
	m_isRunning = true;
	std::cout << "VortexArcana Systems Initialized." << std::endl;
	return true;
}

void Engine::Run() {
	uint64_t lastTime = SDL_GetTicks();

	while (m_isRunning) {
		// Delta Time Calculation.
		uint64_t currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		// Main Loop
		ProcessInput();
		Update(deltaTime);
		Render();
	}
}

void Engine::ProcessInput() {
	SDL_Event event;
	// SDL_PollEvent checks for any present events and puts them in the argument event.
	while (SDL_PollEvent(&event)) {
		// Check for Exit and quit loop if true.
		if (event.type == SDL_EVENT_QUIT) {
			m_isRunning = false;
		}
	}
}

void Engine::Update(float deltaTime) {
	// Physics stuff.
}

void Engine::Render() {
	SDL_SetRenderDrawColor(m_renderer, 15, 15, 20, 255);
	SDL_RenderClear(m_renderer);

	SDL_RenderPresent(m_renderer);
}

void Engine::Shutdown() {
	// Cleanup Function
	if (m_renderer) {
		SDL_DestroyRenderer(m_renderer);
		m_renderer = nullptr;
	}

	if (m_window) {
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}

	SDL_Quit();

	std::cout << "VortexArcana Systems Shutdown Cleanly." << std::endl;
}