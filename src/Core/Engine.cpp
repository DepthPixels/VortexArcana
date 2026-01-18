#define STB_IMAGE_IMPLEMENTATION
#include "Engine.h"
#include <iostream>

Engine::Engine()
	: m_window(nullptr), m_isOpen(false), m_isRunning(false) {
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

	// Set OpenGL Attributes.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Create the SDL Window.
	m_window = SDL_CreateWindow("VortexArcana Engine v0.0.1", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	if (!m_window) {
		std::cerr << "Window Creation Error: " << SDL_GetError() << std::endl;
		return false;
	}

	m_glContext = SDL_GL_CreateContext(m_window);

	// Putting in NULL as name of renderer is basically auto.
	/*
	m_renderer = SDL_CreateRenderer(m_window, NULL);
	if (!m_renderer) {
		std::cerr << "Renderer Creation Error: " << SDL_GetError() << std::endl;
		return false;
	}
	*/

	// Initialize GLAD.
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Viewport Size.
	glViewport(0, 0, 1280, 720);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Texture Stuff
	int width, height, nrChannels;
	unsigned char* data = stbi_load("assets/cloud.jpg", &width, &height, &nrChannels, 4);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	// Generate Texture to the 2D texture target.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Free Image Memory.
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	unsigned char* data2 = stbi_load("assets/yehyafreshman.png", &width, &height, &nrChannels, 4);
	if (data2)
	{
		glGenTextures(1, &m_texture2);
		glBindTexture(GL_TEXTURE_2D, m_texture2);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data2);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Shader basicShader("assets/shaders/basicVertex.vert", "assets/shaders/basicFragment.frag");

	m_shaderProgram = basicShader.ID;

	// Geometry.
	float vertices[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 1.0f,   1.0f, 0.0f,   // top right
		 0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, 1.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   1.0f, 0.5f, 0.5f,   0.0f, 1.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 1.0f,   0.0f, 0.0f    // top left 
	};
	unsigned int indices[] = {
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ebo);

	// Bind VAO first, then bind and set VBO.
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Tells OpenGL how to read the buffer.
	// Positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Colors
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Texture Coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	/*
	m_gameTexture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1280, 720);
	if (m_gameTexture) {
		SDL_SetTextureScaleMode(m_gameTexture, SDL_SCALEMODE_NEAREST);
	}
	*/

	// ImGui setup.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

	ImGui_ImplSDL3_InitForOpenGL(m_window, m_glContext);
	ImGui_ImplOpenGL3_Init("#version 460 core");

	// If nothing failed yet then initialization worked!
	m_isOpen = true;
	std::cout << "VortexArcana Systems Initialized." << std::endl;

	// Make an initial player entity.
	Vortex::Entity player;
	player.mass = 1000.0f;
	//player.SetSprite(m_renderer, "assets/player.bmp");
	player.name = "Player";

	m_entities.push_back(player);

	return true;
}

void Engine::Run() {
	uint64_t lastTime = SDL_GetTicks();

	while (m_isOpen) {
		// Delta Time Calculation.
		uint64_t currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		// Main Loop
		ProcessInput();
		if (m_isRunning) Update(deltaTime);
		Render();
	}
}

void Engine::ProcessInput() {
	SDL_Event event;
	// SDL_PollEvent checks for any present events and puts them in the argument event.
	while (SDL_PollEvent(&event)) {
		// Send events to ImGui.
		ImGui_ImplSDL3_ProcessEvent(&event);

		switch (event.type) {
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button == SDL_BUTTON_LEFT) {
				Vortex::Vec2 clickPos = { event.button.x, event.button.y };
				if (m_viewportRect.isPosInRect(clickPos)) {
					Vortex::Vec2 viewportCoords = CoordsScreenToViewport(clickPos);
					Vortex::Entity* clickedEntity = GetEntityAtViewportCoords(viewportCoords);
					if (clickedEntity) {
						m_isDragging = true;
						for (Vortex::Entity& entity : m_entities) {
							entity.isSelected = false;
							entity.isBeingDragged = false;
						}
						clickedEntity->isSelected = true;
						clickedEntity->isBeingDragged = true;
						m_selectedEntity = clickedEntity;
						m_dragOffset.x = clickedEntity->bounds.position.x - viewportCoords.x;
						m_dragOffset.y = clickedEntity->bounds.position.y - viewportCoords.y;
					}
					else {
						for (Vortex::Entity& entity : m_entities) {
							entity.isSelected = false;
							entity.isBeingDragged = false;
						}
						m_selectedEntity = nullptr;
					}
				}
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event.button.button == SDL_BUTTON_LEFT) {
				Vortex::Vec2 clickPos = { event.button.x, event.button.y };
				if (m_viewportRect.isPosInRect(clickPos)) {
					m_isDragging = false;
					for (Vortex::Entity& entity : m_entities) {
						entity.isBeingDragged = false;
					}
				}
			}
			break;
		case SDL_EVENT_MOUSE_MOTION: {
			Vortex::Vec2 mousePos = { event.motion.x, event.motion.y };
			if (m_isDragging) {
				Vortex::Vec2 viewportCoords = CoordsScreenToViewport(mousePos);
				m_selectedEntity->bounds.position = viewportCoords + m_dragOffset;
				std::cout << "Entity " << m_selectedEntity->name << " Moved to Position (" << m_selectedEntity->bounds.position.x << ", " << m_selectedEntity->bounds.position.y << ")" << std::endl;
			}
			break;
		}
		case SDL_EVENT_QUIT:
			m_isOpen = false;
			break;
		}
	}
}

void Engine::Update(float deltaTime) {
	// Physics stuff.

	Vortex::Vec2 gravityForce = m_gravityDirection * m_gravityStrength * m_entities[0].mass * m_meterToPixel;
	m_entities[0].ApplyForce(gravityForce);

	m_entities[0].Integrate(deltaTime);
}

void Engine::Render() {

	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	glViewport(0, 0, w, h);
	
	// Start the ImGui Frame.
	ImGui_ImplSDL3_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	// ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
	
	// ShowViewportWindow();

	// Show Editor UI.
	// ShowEditorUI();
	
	ImGui::Render();

	glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_shaderProgram);
	glUniform1i(glGetUniformLocation(m_shaderProgram, "texture1"), 0);
	glUniform1i(glGetUniformLocation(m_shaderProgram, "texture2"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_texture2);

	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(m_window);
}

void Engine::ShowViewportWindow() {
	ImGui::Begin("Game Viewport");

	ImGui::BeginDisabled(m_isRunning);
	if (ImGui::Button("Run")) m_isRunning = true;
	ImGui::EndDisabled();
	ImGui::BeginDisabled(!m_isRunning);
	if (ImGui::Button("Stop")) m_isRunning = false;
	ImGui::EndDisabled();

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

	ImVec2 windowPos = ImGui::GetWindowPos();

	ImVec2 viewportPosition = windowPos + ImVec2(offsetX, offsetY);

	m_viewportRect.position.x = ImGui::GetCursorPosX() + viewportPosition.x;
	m_viewportRect.position.y = ImGui::GetCursorPosY() + viewportPosition.y;
	m_viewportRect.w = renderWidth;
	m_viewportRect.h = renderHeight;

	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + offsetX, ImGui::GetCursorPosY() + offsetY));

	ImGui::Image((ImTextureID)m_gameTexture, ImVec2(renderWidth, renderHeight));

	ImGui::End();
}

void Engine::ShowEditorUI() {
	ImGui::Begin("Debug Window");
	ImGui::Text("Mouse Offset: (%.2f, %.2f)", m_dragOffset.x, m_dragOffset.y);
	ImGui::Text("Is Dragging: %s", m_isDragging ? "True" : "False");
	ImGui::End();


	// Entity Inspector
	ImGui::Begin("Entity Inspector");

	if (m_selectedEntity == nullptr) {
		ImGui::Text("Nothing Selected.");
	}
	else {
		ImGui::InputText("Entity Name", &m_selectedEntity->name);

		ImGui::Text("Position");
		ImGui::DragFloat("X", &m_selectedEntity->bounds.position.x, 1.0f, 1.0f);
		ImGui::DragFloat("Y", &m_selectedEntity->bounds.position.y, 1.0f, 1.0f);

		ImGui::DragFloat("Width", &m_selectedEntity->bounds.w, 8.0f, 256.0f);
		ImGui::DragFloat("Height", &m_selectedEntity->bounds.h, 8.0f, 256.0f);
	}

	ImGui::End();
}

void Engine::Shutdown() {
	// Cleanup Function
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteProgram(m_shaderProgram);

	SDL_GL_DestroyContext(m_glContext);

	if (m_window) {
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}

	SDL_Quit();

	std::cout << "VortexArcana Systems Shutdown Cleanly." << std::endl;
}

// Helper Functions
Vortex::Vec2 Engine::CoordsScreenToViewport(Vortex::Vec2 screenCoords) {
	float localX = screenCoords.x - m_viewportRect.position.x;
	float localY = screenCoords.y - m_viewportRect.position.y;

	float scaleX = 1280.0f / m_viewportRect.w;
	float scaleY = 720.0f / m_viewportRect.h;

	return { localX * scaleX, localY * scaleY };
}

Vortex::Entity* Engine::GetEntityAtViewportCoords(Vortex::Vec2 viewportCoords) {
	// Returns the first entity found at the given viewport coordinates.
	for (Vortex::Entity& entity : m_entities) {
		std::cout << "Checking Entity: " << entity.name << " at Pos (" << entity.bounds.position.x << ", " << entity.bounds.position.y << ")" << " and Width (" <<
			entity.bounds.w << ", " << entity.bounds.h << ")" << std::endl;
		std::cout << "Viewport Coords: (" << viewportCoords.x << ", " << viewportCoords.y << ")" << std::endl;
		if (entity.bounds.isPosInRect(viewportCoords)) {
			std::cout << "Clicked on Entity: " << entity.name << std::endl;
			return &entity;
		}
	}
	return nullptr;
}