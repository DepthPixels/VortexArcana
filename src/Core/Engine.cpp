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
	m_window = SDL_CreateWindow("VortexArcana Engine v0.0.1", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
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

	m_gameTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1280, 720);
	if (m_gameTexture) {
		SDL_SetTextureScaleMode(m_gameTexture, SDL_SCALEMODE_NEAREST);
	}

	// ImGui setup.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

	ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
	ImGui_ImplSDLRenderer3_Init(m_renderer);


	// If nothing failed yet then initialization worked!
	m_isRunning = true;
	std::cout << "VortexArcana Systems Initialized." << std::endl;

	m_player.SetSprite(m_renderer, "assets/player.bmp");

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
		// Send events to ImGui.
		ImGui_ImplSDL3_ProcessEvent(&event);

		// Check for Exit and quit loop if true.
		if (event.type == SDL_EVENT_QUIT) {
			m_isRunning = false;
		}
	}
}

bool CheckCollision(Vortex::Rect a, Vortex::Rect b) {
	return (a.x < b.x + b.w &&
		a.x + a.w > b.x &&
		a.y < b.y + b.h &&
		a.y + a.h > b.y);
}

void Engine::Update(float deltaTime) {
	// Physics stuff.

	Vortex::Vec2 gravityForce = m_gravityDirection * m_gravityStrength;
	m_player.ApplyForce(gravityForce);

	m_player.velocity += m_player.acceleration * deltaTime;

	m_player.velocity.x *= 0.95f;

	m_player.position += m_player.velocity * deltaTime;

	m_player.acceleration = { 0, 0 };

	// std::cout << "Collision: " << CheckCollision() << endl
}

void Engine::Render() {
	// Start the ImGui Frame.
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	// Set Render Target to Game Texture.
	SDL_SetRenderTarget(m_renderer, m_gameTexture);

	// Clear Screen.
	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_renderer);

	m_player.Draw(m_renderer);

	// Reset Render Target.
	SDL_SetRenderTarget(m_renderer, NULL);

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
	ShowViewportWindow();

	// Show Editor UI.
	ShowEditorUI();

	SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_renderer);

	ImGui::Render();

	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);

	SDL_RenderPresent(m_renderer);
}

void Engine::ShowViewportWindow() {
	ImGui::Begin("Game Viewport");

	// Get Window Size
	ImVec2 windowSize = ImGui::GetContentRegionAvail();

	float targetAspectRatio = 1280.0f / 720.0f;
	float renderWidth = windowSize.x;
	float renderHeight = renderWidth / targetAspectRatio;

	if (renderHeight > windowSize.y) {
		renderHeight = windowSize.y;
		renderWidth = renderHeight * targetAspectRatio;
	}

	float offsetX = (windowSize.x - renderWidth) * 0.5f;
	float offsetY = (windowSize.y - renderHeight) * 0.5f;

	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + offsetX, ImGui::GetCursorPosY() + offsetY));

	ImGui::Image((ImTextureID)m_gameTexture, ImVec2(renderWidth, renderHeight));

	ImGui::End();
}

void Engine::ShowEditorUI() {
	ImGui::Begin("Vortex Physics");
	ImGui::Text("Player Pos: (%.2f, %.2f)", m_player.position.x, m_player.position.y);
	ImGui::Text("Player Vel: (%.2f, %.2f)", m_player.velocity.x, m_player.velocity.y);
	ImGui::End();

	ImGui::Begin("Entity Inspector");
	ImGui::Text("Player Sprite: %s", m_player.sprite ? "Loaded" : "Missing");

	ImGui::DragFloat("Width", &m_player.width, 16.0f, 256.0f);
	ImGui::DragFloat("Height", &m_player.height, 16.0f, 256.0f);

	ImGui::End();
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