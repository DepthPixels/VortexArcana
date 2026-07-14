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

	// Initialize Bridge to .NET for Scripting.
	init_bridge(STR("F:\\Cpp\\VortexArcana\\out\\build\\windows-vs2026\\Debug\\"));

	Vortex::Entity* object1 = new Vortex::Entity();
	object1->name = "Yehya Freshman Sprite";
	object1->bounds = { 300.0f, 200.0f, 200.0f, 200.0f };
	Vortex::SpriteRenderer2D* spriteComponent = new Vortex::SpriteRenderer2D;
	spriteComponent->LoadSprite("assets/yehyafreshman.png", true);
	object1->AddComponent(spriteComponent);
	Vortex::Physics2D* physics2D = new Vortex::Physics2D();
	physics2D->Mass() = 1000.0f;
	object1->AddComponent(physics2D);
	m_entities.push_back(object1);	

	Vortex::Entity* object2 = new Vortex::Entity();
	object2->name = "Spotlight";
	object2->bounds = { 100.0f, 200.0f, 100.0f, 100.0f };
	Vortex::PointLight* ptlght = new Vortex::PointLight();
	object2->AddComponent(ptlght);
	m_entities.push_back(object2);

	// Camera Stuff
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);

	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 directionToCamera = glm::normalize(cameraPos - cameraTarget);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, directionToCamera));
	glm::vec3 cameraUp = glm::cross(directionToCamera, cameraRight);

	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, up);

	m_currentViewMatrix = view;

	m_compositionShader = new Shader("assets/shaders/compositionVertex.glsl", "assets/shaders/compositionFragment.glsl");

	// Setting up FBO.
	glGenFramebuffers(1, &m_viewport_fbo);
	glGenFramebuffers(1, &m_albedo_fbo);
	glGenFramebuffers(1, &m_ambient_fbo);
	glGenFramebuffers(1, &m_occlusion_fbo);

	glBindFramebuffer(GL_FRAMEBUFFER, m_viewport_fbo);
	glGenTextures(1, &m_viewportTexture);
	glBindTexture(GL_TEXTURE_2D, m_viewportTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)m_viewportSize.x, (int)m_viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_viewportTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, m_albedo_fbo);
	glGenTextures(1, &m_albedoTexture);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)m_viewportSize.x, (int)m_viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_albedoTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, m_ambient_fbo);
	glGenTextures(1, &m_ambientTexture);
	glBindTexture(GL_TEXTURE_2D, m_ambientTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)m_viewportSize.x, (int)m_viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ambientTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, m_occlusion_fbo);
	glGenTextures(1, &m_occlusionTexture);
	glBindTexture(GL_TEXTURE_2D, m_occlusionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)m_viewportSize.x, (int)m_viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_occlusionTexture, 0);

	// Render Buffer Objects
	glBindFramebuffer(GL_FRAMEBUFFER, m_albedo_fbo);
	glGenRenderbuffers(1, &m_albedo_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_albedo_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)m_viewportSize.x, (int)m_viewportSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_albedo_rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, m_occlusion_rbo);
	glGenRenderbuffers(1, &m_occlusion_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_occlusion_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)m_viewportSize.x, (int)m_viewportSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_occlusion_rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	// Unbind.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// Simple Square for the composition
	float quadVertices[] = {
		// Positions   // TexCoords
		-1.0f,  1.0f,  0.0f, 1.0f, // Top Left
		-1.0f, -1.0f,  0.0f, 0.0f, // Bottom Left
		 1.0f, -1.0f,  1.0f, 0.0f, // Bottom Right

		-1.0f,  1.0f,  0.0f, 1.0f, // Top Left
		 1.0f, -1.0f,  1.0f, 0.0f, // Bottom Right
		 1.0f,  1.0f,  1.0f, 1.0f  // Top Right
	};

	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo);

	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	// Position Attribute (X, Y)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	// Texture Coordinate Attribute (U, V)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0); // Unbind

	m_chosenDisplayMode = DisplayMode::Combined;

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
					if (m_tilingMode) {
						Vortex::Vec2 tilePlaceCoords = Vortex::Vec2(int(viewportCoords.x) - (int(viewportCoords.x) % 32), int(viewportCoords.y) - (int(viewportCoords.y) % 32));
						Vortex::Entity* newTile = new Vortex::Tile(tilePlaceCoords, m_selectedTileLocation);
						m_entities.push_back(newTile);
					}
					else {
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
			}
			if (event.button.button == SDL_BUTTON_MIDDLE) {
				Vortex::Vec2 clickPos = { event.button.x, event.button.y };
				if (m_viewportRect.isPosInRect(clickPos)) {
					if (m_selectedEntity != nullptr) {
						m_selectedEntity->isSelected = false;
						m_selectedEntity->isBeingDragged = false;
						m_selectedEntity = nullptr;
					}
					m_isDragging = true;
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
			if (event.button.button == SDL_BUTTON_MIDDLE) {
				Vortex::Vec2 clickPos = { event.button.x, event.button.y };
				if (m_viewportRect.isPosInRect(clickPos)) {
					m_isDragging = false;
				}
			}
			break;
		case SDL_EVENT_MOUSE_MOTION: {
			Vortex::Vec2 mousePos = { event.motion.x, event.motion.y };
			if (m_isDragging) {
				if (m_selectedEntity != nullptr) {
					Vortex::Vec2 viewportCoords = CoordsScreenToViewport(mousePos);
					m_selectedEntity->bounds.position = viewportCoords + m_dragOffset;
				}
				else {
					// Dragging Viewport
					m_currentViewMatrix = glm::translate(m_currentViewMatrix, glm::vec3(event.motion.xrel, event.motion.yrel, 0.0f));
				}
			}
			break;
		}
		case SDL_EVENT_KEY_DOWN:
			if (event.key.key == SDLK_ESCAPE) {
				m_tilingMode = false;
				m_selectedTileLocation = "None";
			}
			else if (event.key.key == SDLK_W) {
				m_currentViewMatrix = glm::translate(m_currentViewMatrix, glm::vec3(0.0f, -10.0f, 0.0f));
			}
			else if (event.key.key == SDLK_A) {
				m_currentViewMatrix = glm::translate(m_currentViewMatrix, glm::vec3(-10.0f, 0.0f, 0.0f));
			}
			else if (event.key.key == SDLK_S) {
				m_currentViewMatrix = glm::translate(m_currentViewMatrix, glm::vec3(0.0f, 10.0f, 0.0f));
			}
			else if (event.key.key == SDLK_D) {
				m_currentViewMatrix = glm::translate(m_currentViewMatrix, glm::vec3(10.0f, 0.0f, 0.0f));
			}
			break;
		case SDL_EVENT_QUIT:
			m_isOpen = false;
			break;
		}
	}
}

void Engine::Update(float deltaTime) {
	// Physics stuff.
	Vortex::Vec2 gravityForce = m_gravityDirection * m_gravityStrength * m_meterToPixel;

	update_all_scripts_through_bridge();
	phys_update_all_scripts_through_bridge(deltaTime);

	for (Vortex::Entity* entity : m_entities) {
		Vortex::Physics2D* physics2D = entity->GetComponent<Vortex::Physics2D>();
		if (physics2D) {
			physics2D->ApplyForce(gravityForce * physics2D->Mass());
		}
		entity->UpdateComponents(deltaTime);
	}
}

void Engine::Render() {

	// Bind to Occlusion FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, m_occlusion_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Game Resolution.
	glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

	// Draw to Occlusion FBO
	for (Vortex::Entity* entity : m_entities) {
		entity->RenderOcclusion(m_currentViewMatrix);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Bind to Albedo FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, m_albedo_fbo);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Stencil Buffer stuff.
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	// Draw to Albedo FBO
	for (Vortex::Entity* entity : m_entities) {
		entity->RenderAlbedo(m_currentViewMatrix);
	}

	// Unbind FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Bind to Ambient FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, m_ambient_fbo);
	glBlendFunc(GL_ONE, GL_ONE);

	glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, m_occlusionTexture);

	// Draw to Ambient FBO
	for (Vortex::Entity* entity : m_entities) {
		entity->RenderLights(m_currentViewMatrix);
	}

	// Unbind FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Bind to Viewport FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, m_viewport_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_ambientTexture);

	m_compositionShader->use();
	m_compositionShader->setInt("albedoTextureID", 0);
	m_compositionShader->setInt("ambientTextureID", 1);

	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

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

	ImGui::Text("Render Mode");
	if (ImGui::Button("Combined")) m_chosenDisplayMode = DisplayMode::Combined;
	if (ImGui::Button("Albedo")) m_chosenDisplayMode = DisplayMode::Albedo;
	if (ImGui::Button("Ambient")) m_chosenDisplayMode = DisplayMode::Ambient;
	if (ImGui::Button("Occlusion")) m_chosenDisplayMode = DisplayMode::Occlusion;

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

	ImTextureID chosenTexture;

	switch (m_chosenDisplayMode) {
		case DisplayMode::Combined:
			chosenTexture = (ImTextureID)(intptr_t)m_viewportTexture;
			break;
		case DisplayMode::Albedo:
			chosenTexture = (ImTextureID)(intptr_t)m_albedoTexture;
			break;
		case DisplayMode::Ambient:
			chosenTexture = (ImTextureID)(intptr_t)m_ambientTexture;
			break;
		case DisplayMode::Occlusion:
			chosenTexture = (ImTextureID)(intptr_t)m_occlusionTexture;
			break;
	}

	ImGui::Image(chosenTexture, ImVec2(renderWidth, renderHeight), ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
}

void Engine::ShowEditorUI() {
	// Debug Window
	ImGui::Begin("Debug Window");
	ImGui::Text("Mouse Offset: (%.2f, %.2f)", m_dragOffset.x, m_dragOffset.y);
	ImGui::Text("Is Dragging: %s", m_isDragging ? "True" : "False");
	ImGui::Text("Is Selected: %s", (m_selectedEntity != nullptr) ? (m_selectedEntity->isSelected) ? "True" : "False" : "Nothing Selected");
	ImGui::Text("Tiling Mode: %s", (m_tilingMode) ? "True" : "False");
	ImGui::Text("Selected Tile Location: %s", (m_tilingMode) ? m_selectedTileLocation.c_str() : "Tiling Mode Off");
	if (ImGui::Button("Reset Tile Location")) {
		m_selectedTileLocation = "None";
	}
	if (ImGui::Button("Update All Scripts")) {
		update_all_scripts_through_bridge();
	}
	ImGui::End();

	// Working Tree
	ImGui::Begin("Working Tree");
	ImGui::BeginDisabled(m_isRunning);
	ImGui::InputText("Scene Name", &this->currentScene);
	if (ImGui::Button("Save Scene")) Vortex::SceneUtility::SaveScene(this->m_entities, "assets/scenes/" + this->currentScene + ".scene");
	if (ImGui::Button("Load Default Scene")) Vortex::SceneUtility::LoadScene(this->m_entities, "assets/scenes/Default Scene.scene");
	ImGui::EndDisabled();
	for (Vortex::Entity* entity : m_entities) {
		ImGui::PushID(entity->EntityID());
		if (ImGui::Button(("- %s", entity->name.c_str()))) {
			m_selectedEntity = entity;
		}
		ImGui::PopID();
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
				Vortex::Physics2D* physics = dynamic_cast<Vortex::Physics2D*>(component);
				if (physics) {
					ImGui::Text("- Physics 2D");
					ImGui::DragFloat("Mass", &physics->Mass(), 1.0f, 0.1f, 10000.0f);
					ImGui::Text("Velocity: (%.2f, %.2f)", physics->Velocity().x, physics->Velocity().y);
				}
				Vortex::PointLight* pointlight = dynamic_cast<Vortex::PointLight*>(component);
				if (pointlight) {
					ImGui::Text("- Point Light");
					ImGui::DragFloat("Brightness", &pointlight->brightness);
					ImGui::DragFloat("Falloff", &pointlight->falloff);
					ImGui::DragFloat("Radius", &pointlight->radius);
					ImGui::ColorEdit3("Color", (float*)&pointlight->color);
				}
			}
		}

		if (ImGui::Button("Add Component")) {
			ImGui::OpenPopup("AddComponentPopup");
		}

		// Add Component Popup
		if (ImGui::BeginPopup("AddComponentPopup")) {
			if (ImGui::Selectable("Sprite Renderer 2D")) {
				Vortex::SpriteRenderer2D* spriteComponent = new Vortex::SpriteRenderer2D;
				m_selectedEntity->AddComponent(spriteComponent);
				std::cout << "Added SpriteRenderer2D Component to " << m_selectedEntity->name << std::endl;
			}
			if (ImGui::Selectable("Physics 2D")) {
				Vortex::Physics2D* physicsComponent = new Vortex::Physics2D();
				m_selectedEntity->AddComponent(physicsComponent);
				std::cout << "Added Physics2D Component to " << m_selectedEntity->name << std::endl;
			}
			if (ImGui::Selectable("Point Light")) {
				Vortex::PointLight* pointLightComponent = new Vortex::PointLight();
				m_selectedEntity->AddComponent(pointLightComponent);
				std::cout << "Added PointLight Component to " << m_selectedEntity->name << std::endl;
			}
			ImGui::EndPopup();
		}


		ImGui::Text("Scripts");
		for (const auto& script : m_selectedEntity->scriptsAttached) {
			ImGui::Text("- %s", script.c_str());
		}

		if (ImGui::Button("Add Script")) {
			ImGui::OpenPopup("AddScriptPopup");
		}

		if (ImGui::BeginPopup("AddScriptPopup")) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator("DebugScripts/")) {
				if (entry.is_regular_file()) {
					if (entry.path().string().ends_with(".cs")) {
						if (ImGui::Selectable(entry.path().filename().string().c_str())) {
							// Script was selected
							if (m_selectedEntity->scriptsAttached.count(entry.path().filename().string())) {
								std::cout << "Script: " << entry.path().filename().string() << " is already attached to " << m_selectedEntity->name << std::endl;
							}
							else {
								m_selectedEntity->scriptsAttached.insert(entry.path().filename().string());
								instantiate_script_through_bridge(m_selectedEntity->EntityID(), entry.path().filename());
								std::cout << "Added Script: " << entry.path().filename().string() << " to " << m_selectedEntity->name << std::endl;
							}
						}
					}
				}
			}
			ImGui::EndPopup();
		}
	}
	ImGui::End();

	// Tile Manager
	ImGui::Begin("Tile Manager");
	std::string newTileLocationBuffer;
	ImGui::InputText("New Tile Location: ", &newTileLocationBuffer);
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		if (newTileLocationBuffer != "None" && newTileLocationBuffer != "") {
			m_tileLocations.push_back(newTileLocationBuffer);
			std::cout << "Added Tile Location: " << newTileLocationBuffer << std::endl;
		}
	}
	for (std::string location : m_tileLocations) {
		if (ImGui::Button(location.c_str())) {
			m_selectedTileLocation = location;
		}
	}
	ImGui::End();
}

void Engine::Shutdown() {
	// Cleanup Function
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);

	// Close Bridge
	exit_bridge();

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
		if (entity->bounds.isPosInRect(viewportCoords)) {
			return entity;
		}
	}
	return nullptr;
}