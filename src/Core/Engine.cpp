#include "Engine.h"
#include <iostream>

Engine::Engine()
	: m_window(nullptr), m_isOpen(false), m_isRunning(false), m_basicShader(nullptr) {
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
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	// Create the SDL Window.
	m_window = SDL_CreateWindow("VortexArcana Engine v0.0.1", 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	if (!m_window) {
		std::cerr << "Window Creation Error: " << SDL_GetError() << std::endl;
		return false;
	}

	m_glContext = SDL_GL_CreateContext(m_window);

	// Initialize GLAD.
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// Viewport Size.
	glViewport(0, 0, 1280, 720);

	glEnable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Texture Stuff
	m_basicShader = new Shader("assets/shaders/basicVertex.glsl", "assets/shaders/basicFragment.glsl");

	glm::mat4 projection = glm::ortho(0.0f, m_viewportSize.x, m_viewportSize.y, 0.0f, -1.0f, 1.0f);

	m_basicShader->use();
	m_basicShader->setMat4("projection", projection);

	Vortex::Entity* object1 = new Vortex::Entity();
	object1->name = "Yehya Freshman Sprite";
	object1->bounds = { 300.0f, 200.0f, 200.0f, 200.0f };
	Vortex::SpriteRenderer2D* spriteComponent = new Vortex::SpriteRenderer2D(m_basicShader);
	spriteComponent->LoadSprite("assets/yehyafreshman.png", true);
	object1->AddComponent(spriteComponent);
	Vortex::Physics2D* physics2D = new Vortex::Physics2D();
	physics2D->mass = 1000.0f;
	object1->AddComponent(physics2D);
	m_entities.push_back(object1);

	// Setting up FBO.
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_viewportTexture);
	glBindTexture(GL_TEXTURE_2D, m_viewportTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)m_viewportSize.x, (int)m_viewportSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_viewportTexture, 0);

	// Render Buffer Object
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)m_viewportSize.x, (int)m_viewportSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	// Unbind.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
						for (Vortex::Entity* entity : m_entities) {
							entity->isSelected = false;
							entity->isBeingDragged = false;
						}
						clickedEntity->isSelected = true;
						clickedEntity->isBeingDragged = true;
						m_selectedEntity = clickedEntity;
						m_dragOffset.x = clickedEntity->bounds.position.x - viewportCoords.x;
						m_dragOffset.y = clickedEntity->bounds.position.y - viewportCoords.y;
					}
					else {
						for (Vortex::Entity* entity : m_entities) {
							entity->isSelected = false;
							entity->isBeingDragged = false;
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
					for (Vortex::Entity* entity : m_entities) {
						entity->isBeingDragged = false;
					}
				}
			}
			break;
		case SDL_EVENT_MOUSE_MOTION: {
			Vortex::Vec2 mousePos = { event.motion.x, event.motion.y };
			if (m_isDragging) {
				Vortex::Vec2 viewportCoords = CoordsScreenToViewport(mousePos);
				m_selectedEntity->bounds.position = viewportCoords + m_dragOffset;
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
	Vortex::Vec2 gravityForce = m_gravityDirection * m_gravityStrength * m_meterToPixel;
	
	for (Vortex::Entity* entity : m_entities) {
		Vortex::Physics2D* physics2D = entity->GetComponent<Vortex::Physics2D>();
		if (physics2D) {
			physics2D->ApplyForce(gravityForce * physics2D->mass);
		}
		entity->UpdateComponents(deltaTime);
	}
}

void Engine::Render() {

	// Render Game to FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	// Game Resolution.
	glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);
	glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Stencil Buffer stuff.
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// Draw Entities.
	for (Vortex::Entity* entity : m_entities) {
		entity->RenderComponents();
	}

	// Unbind FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);
	glViewport(0, 0, w, h);

	glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Start the ImGui Frame.
	ImGui_ImplSDL3_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
	
	ShowViewportWindow();

	// Show Editor UI.
	ShowEditorUI();
	
	ImGui::Render();

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

	ImGui::Image((ImTextureID)(uintptr_t)m_viewportTexture, ImVec2(renderWidth, renderHeight), ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
}

void Engine::ShowEditorUI() {
	ImGui::Begin("Debug Window");
	ImGui::Text("Mouse Offset: (%.2f, %.2f)", m_dragOffset.x, m_dragOffset.y);
	ImGui::Text("Is Dragging: %s", m_isDragging ? "True" : "False");
	ImGui::Text("Is Selected: %s", (m_selectedEntity != nullptr) ? (m_selectedEntity->isSelected) ? "True" : "False" : "Nothing Selected");
	ImGui::End();

	// Working Tree
	ImGui::Begin("Working Tree");
	for (Vortex::Entity* entity : m_entities) {
		if (ImGui::Button(("- %s", entity->name.c_str()))) {
			m_selectedEntity = entity;
		}
	}
	if (ImGui::Button("Add Entity")) {
		Vortex::Entity* newEntity = new Vortex::Entity();
		newEntity->bounds.position = { 100.0f, 100.0f };
		newEntity->name = "New Entity";
		m_entities.push_back(newEntity);
	}
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

		ImGui::Text("Components");
		if (m_selectedEntity->GetAllComponents().size() != 0) {
			for (Vortex::Component* component : m_selectedEntity->GetAllComponents()) {
				Vortex::SpriteRenderer2D* sprite = dynamic_cast<Vortex::SpriteRenderer2D*>(component);
				if (sprite) {
					ImGui::Text("- Sprite Renderer 2D");
					std::string spriteLocationBuffer = sprite->spriteLocation;
					std::string oldStringLocation = spriteLocationBuffer;
					ImGui::InputText("Sprite Location", &spriteLocationBuffer);
					if (ImGui::IsItemDeactivatedAfterEdit() && (spriteLocationBuffer != oldStringLocation)) {
						sprite->LoadSprite(spriteLocationBuffer.c_str(), true);
						oldStringLocation = spriteLocationBuffer;
					}
				}
			}
		}
		else {
			if (ImGui::Button("Add Component")) {
				ImGui::OpenPopup("AddComponentPopup");
			}
		}

		// Add Component Popup
		if (ImGui::BeginPopup("AddComponentPopup")) {
			if (ImGui::Selectable("Sprite Renderer 2D")) {
				Vortex::SpriteRenderer2D* spriteComponent = new Vortex::SpriteRenderer2D(m_basicShader);
				m_selectedEntity->AddComponent(spriteComponent);
			}
			if (ImGui::Selectable("Physics 2D")) {
				Vortex::Physics2D* physicsComponent = new Vortex::Physics2D();
				m_selectedEntity->AddComponent(physicsComponent);
			}
			ImGui::EndPopup();
		}
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
	for (Vortex::Entity* entity : m_entities) {
		std::cout << "Checking Entity: " << entity->name << " at Pos (" << entity->bounds.position.x << ", " << entity->bounds.position.y << ")" << " and Width (" <<
			entity->bounds.w << ", " << entity->bounds.h << ")" << std::endl;
		std::cout << "Viewport Coords: (" << viewportCoords.x << ", " << viewportCoords.y << ")" << std::endl;
		if (entity->bounds.isPosInRect(viewportCoords)) {
			std::cout << "Clicked on Entity: " << entity->name << std::endl;
			return entity;
		}
	}
	return nullptr;
}