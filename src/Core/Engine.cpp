#include "Engine.h"
#include <iostream>
#include <algorithm>
#include <utility>


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

	/* Light Test Setup
	Vortex::Entity* object1 = new Vortex::Entity();
	object1->name = "Yehya Freshman Sprite";
	object1->bounds = { 300.0f, 200.0f, 200.0f, 200.0f };
	Vortex::SpriteRenderer2D* spriteComponent = new Vortex::SpriteRenderer2D;
	object1->AddComponent(spriteComponent);
	spriteComponent->LoadSprite("assets/LightTest.png", true);
	Vortex::Physics2D* physics2D = new Vortex::Physics2D();
	physics2D->Mass() = 1000.0f;
	object1->AddComponent(physics2D);
	m_entities.push_back(object1);	

	Vortex::Entity* object2 = new Vortex::Entity();
	object2->name = "Spotlight";
	object2->bounds = { 100.0f, 200.0f, 100.0f, 100.0f };
	Vortex::PointLight* ptlght = new Vortex::PointLight();
	object2->AddComponent(ptlght);
	//object2->SetParent(object1);
	m_entities.push_back(object2);

	Vortex::Entity* object3 = new Vortex::Entity();
	object3->name = "Spotlight 2";
	object3->bounds = { 200.0f, 100.0f, 100.0f, 100.0f };
	Vortex::PointLight* ptlght2 = new Vortex::PointLight();
	object3->AddComponent(ptlght2);
	m_entities.push_back(object3);
	*/

	Vortex::Entity* object1 = new Vortex::Entity();
	object1->name = "Box 1";
	object1->bounds = { 100.0f, 100.0f, 200.0f, 200.0f };
	Vortex::SpriteRenderer2D* spriteComponent = new Vortex::SpriteRenderer2D;
	object1->AddComponent(spriteComponent);
	spriteComponent->LoadSprite("assets/Circle.png", true);
	Vortex::Physics2D* physics2D = new Vortex::Physics2D();
	physics2D->Mass() = 1000.0f;
	object1->AddComponent(physics2D);
	Vortex::Rigidbody* rigidbody = new Vortex::Rigidbody();
	object1->AddComponent(rigidbody);
	rigidbody->Initialize(Vortex::CollisionShape::Circle);
	rigidbody->collider->originOffset = { 100.0f, 100.0f };
	reinterpret_cast<Vortex::CircleCollisionContainer*>(rigidbody->collider)->radius = 100.0f;
	m_entities.push_back(object1);

	Vortex::Entity* object2 = new Vortex::Entity();
	object2->name = "Box 2";
	object2->bounds = { 100.0f, 350.0f, 200.0f, 200.0f };
	Vortex::SpriteRenderer2D* sprite2Component = new Vortex::SpriteRenderer2D;
	object2->AddComponent(sprite2Component);
	sprite2Component->LoadSprite("assets/Slope.png", true);
	//Vortex::Physics2D* physics2D2 = new Vortex::Physics2D();
	//physics2D2->Mass() = 1000.0f;
	//object2->AddComponent(physics2D2);
	Vortex::Rigidbody* rigidbody2 = new Vortex::Rigidbody();
	object2->AddComponent(rigidbody2);
	rigidbody2->Initialize(Vortex::CollisionShape::StaticOcclusion);
	reinterpret_cast<Vortex::StaticOcclusionCollisionContainer*>(rigidbody2->collider)->BakeCollisionMask();
	reinterpret_cast<Vortex::StaticOcclusionCollisionContainer*>(rigidbody2->collider)->LoadBufferFromTexture();
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
	m_gaussBlurShader = new Shader("assets/shaders/GaussianVertex.glsl", "assets/shaders/GaussianFragment.glsl");
	m_lightingShader = new Shader("assets/shaders/pointLightVertex.glsl", "assets/shaders/pointLightFragmentAlternate.glsl");

	// Setting up SSBO.
	m_lightData.resize(160); // Reserve space for 20 lights.

	glGenBuffers(1, &m_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);

	size_t bufferSize = m_lightData.size() * sizeof(float);

	glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STREAM_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Setting up FBO.
	glGenFramebuffers(1, &m_viewport_fbo);
	glGenFramebuffers(1, &m_albedo_fbo);
	glGenFramebuffers(1, &m_ambient_fbo);
	glGenFramebuffers(1, &m_occlusion_fbo);
	glGenFramebuffers(1, &m_blur_fbo1);
	glGenFramebuffers(1, &m_blur_fbo2);

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

	glBindFramebuffer(GL_FRAMEBUFFER, m_blur_fbo1);
	glGenTextures(1, &m_blurTexture1);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)m_viewportSize.x / 2, (int)m_viewportSize.y / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blurTexture1, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, m_blur_fbo2);
	glGenTextures(1, &m_blurTexture2);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)m_viewportSize.x / 2, (int)m_viewportSize.y / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blurTexture2, 0);

	// Render Buffer Objects
	glBindFramebuffer(GL_FRAMEBUFFER, m_albedo_fbo);
	glGenRenderbuffers(1, &m_albedo_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_albedo_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (int)m_viewportSize.x, (int)m_viewportSize.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_albedo_rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, m_occlusion_fbo);
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

	bool prevRunning = false;

	while (m_isOpen) {
		// Delta Time Calculation.
		uint64_t currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		// Main Loop
		ProcessInput();

		if (m_isRunning && prevRunning) Awake();
		if (m_isRunning) Update(deltaTime);

		prevRunning = m_isRunning;

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
							m_dragOffset.x = clickedEntity->GetPosition().x - viewportCoords.x;
							m_dragOffset.y = clickedEntity->GetPosition().y - viewportCoords.y;
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
					m_selectedEntity->SetPosition(viewportCoords + m_dragOffset);
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
			break;
		case SDL_EVENT_QUIT:
			m_isOpen = false;
			break;
		}
	}
}

void Engine::Awake() {
	for (Vortex::Entity* entity : m_entities) {
		Vortex::Rigidbody* rigidbody = entity->GetComponent<Vortex::Rigidbody>();

		if (rigidbody && rigidbody->collider->GetCollisionShapeType() == Vortex::CollisionShape::StaticOcclusion) {
			Vortex::StaticOcclusionCollisionContainer* collider = reinterpret_cast<Vortex::StaticOcclusionCollisionContainer*>(rigidbody->collider);
			collider->BakeCollisionMask();
			collider->LoadBufferFromTexture();
		}
	}
}

void Engine::Update(float deltaTime) {
	// Physics stuff.
	Vortex::Vec2 gravityForce = m_gravityDirection * m_gravityStrength * m_meterToPixel;

	update_all_scripts_through_bridge();
	phys_update_all_scripts_through_bridge(deltaTime);

	// Collisions
	for (size_t i = 0; i < m_entities.size(); ++i) {
		Vortex::Entity* entity = m_entities[i];
		Vortex::Rigidbody* rigidbody = entity->GetComponent<Vortex::Rigidbody>();

		if (rigidbody) {
			for (size_t j = i + 1; j < m_entities.size(); ++j) {
				Vortex::Entity* otherEntity = m_entities[j];

				Vortex::Rigidbody* otherbody = otherEntity->GetComponent<Vortex::Rigidbody>();
				if (otherbody) {
					Vortex::CollisionManifold collisionData = rigidbody->checkCollision(otherbody);

					if (collisionData.isColliding) {

						Vortex::Physics2D* physA = entity->GetComponent<Vortex::Physics2D>();
						Vortex::Physics2D* physB = otherEntity->GetComponent<Vortex::Physics2D>();

						// Denominator
						float invMassA = physA ? (1.0f / physA->Mass()) : 0.0f;
						float invMassB = physB ? (1.0f / physB->Mass()) : 0.0f;
						float totalInvMass = invMassA + invMassB;

						if (totalInvMass <= 0.0f) continue; // Both are static

						// fixing penetration
						float percent = 0.8f;
						float slop = 0.01f;
						float correctionMag = (std::max)(collisionData.penetration - slop, 0.0f) / totalInvMass * percent;
						Vortex::Vec2 correction = collisionData.normal * correctionMag;

						if (physA) entity->SetPosition(entity->GetPosition() - (correction * invMassA));
						if (physB) otherEntity->SetPosition(otherEntity->GetPosition() + (correction * invMassB));

						// Impulse Resolution
						if (physA && physB) {
							std::cout << "Dynamic Collision!" << std::endl;
							Vortex::Vec2 Vrel = physB->Velocity() - physA->Velocity();
							float velAlongNormal = Vrel.dot(collisionData.normal);

							if (velAlongNormal < 0) {
								float j_impulse = -(1.0f + m_bounciness) * velAlongNormal;
								j_impulse /= totalInvMass;

								Vortex::Vec2 impulse = collisionData.normal * j_impulse;

								physA->Velocity() -= (impulse * invMassA);
								physB->Velocity() += (impulse * invMassB);
							}
						}
						else if (physA && !physB) {
							std::cout << "Static Collision!" << std::endl;
							float velAlongNormal = physA->Velocity().dot(collisionData.normal);
							if (velAlongNormal > 0) {
								float j_impulse = -(1.0f + m_bounciness) * velAlongNormal;
								j_impulse /= invMassA;
								physA->Velocity() += (collisionData.normal * j_impulse * invMassA);
							}
						}
						else if (!physA && physB) {
							std::cout << "Static Collision!" << std::endl;
							float velAlongNormal = physB->Velocity().dot(collisionData.normal);
							if (velAlongNormal < 0) {
								float j_impulse = -(1.0f + m_bounciness) * velAlongNormal;
								j_impulse /= invMassB;
								physB->Velocity() += (collisionData.normal * j_impulse * invMassB);
							}
						}
					}
				}
			}
		}
	}

	// gravity if enabled
	if (m_gravity) {
		for (Vortex::Entity* entity : m_entities) {
			Vortex::Physics2D* physics2D = entity->GetComponent<Vortex::Physics2D>();
			if (physics2D) {
				physics2D->ApplyForce(gravityForce * physics2D->Mass());
			}
		}
	}

	// update components
	for (Vortex::Entity* entity : m_entities) {
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

	glEnable(GL_BLEND);

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
	glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);

	int lightCount = 0;

	m_lightData.clear();

	// Collect Light Positions for SSBO.
	for (Vortex::Entity* entity : m_entities) {
		entity->RenderLights(m_lightData, lightCount);
	}

	size_t bufferSize = m_lightData.size() * sizeof(Vortex::Vec2);

	// Activate Shader Program.
	m_lightingShader->use();
	GLint currentProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
	if (currentProgram != m_lightingShader->ID) {
		std::cout << "Shader binding failed!" << std::endl;
	}
	m_lightingShader->setInt("lightCount", lightCount);
	m_lightingShader->setFloat("shadowFalloff", m_shadowFalloff);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);

	// Upload the padded data
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_lightData.size() * sizeof(Vortex::Vec2), m_lightData.data());
	
	GLenum err = glGetError();
	while (err != GL_NO_ERROR) {
		std::cerr << "OpenGL Error: " << err << std::endl;
		err = glGetError();
	}

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	m_lightingShader->use();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, m_occlusionTexture);

	// Bind VAO, Draw, then Unbind.
	glBindVertexArray(m_vao);

	GLint boundBuffer;
	glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 0, &boundBuffer);
	if (boundBuffer != (GLint)m_ssbo) {
		std::cerr << "SSBO not bound to index 0!" << std::endl;
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);


	// Unbind FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// Shadow Blur Horizontal Pass
	glBindFramebuffer(GL_FRAMEBUFFER, m_blur_fbo1);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);

	glViewport(0, 0, m_viewportSize.x / 2, m_viewportSize.y / 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_ambientTexture);

	m_gaussBlurShader->use();
	m_gaussBlurShader->setSampler2D("u_image", 0);
	m_gaussBlurShader->setVec2("u_resolution", m_viewportSize);
	m_gaussBlurShader->setFloat("u_blurScale", m_blurScale);
	// Horizontal Gaussian
	m_gaussBlurShader->setVec2("u_dir", Vortex::Vec2(1.0, 0.0));
	m_gaussBlurShader->setFloat("u_radius", m_viewportSize.x);

	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Shadow Blur Vertical Pass
	glBindFramebuffer(GL_FRAMEBUFFER, m_blur_fbo2);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture1);

	m_gaussBlurShader->use();
	m_gaussBlurShader->setSampler2D("u_image", 0);
	m_gaussBlurShader->setVec2("u_resolution", m_viewportSize / 2);
	m_gaussBlurShader->setFloat("u_blurScale", m_blurScale);
	// Horizontal Gaussian
	m_gaussBlurShader->setVec2("u_dir", Vortex::Vec2(0.0, 1.0));
	m_gaussBlurShader->setFloat("u_radius", m_viewportSize.y / 2);

	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Bind to Viewport FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, m_viewport_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_albedoTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture2);

	m_compositionShader->use();
	m_compositionShader->setSampler2D("albedoTextureID", 0);
	m_compositionShader->setSampler2D("ambientTextureID", 1);

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
	if (ImGui::Button("Blur Pass 1")) m_chosenDisplayMode = DisplayMode::BlurPass1;
	if (ImGui::Button("Blur Pass 2")) m_chosenDisplayMode = DisplayMode::BlurPass2;
	ImGui::DragFloat("Blur Scale", &m_blurScale, 0.1f, 0.1f, 10.0f);
	ImGui::DragFloat("Shadow Falloff", &m_shadowFalloff, 0.1f, 0.1f, 10.0f);

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
		case DisplayMode::BlurPass1:
			chosenTexture = (ImTextureID)(intptr_t)m_blurTexture1;
			break;
		case DisplayMode::BlurPass2:
			chosenTexture = (ImTextureID)(intptr_t)m_blurTexture2;
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
		newEntity->SetPosition(100.0f, 100.0f);
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

		if (ImGui::Button("Parent")) {
			ImGui::OpenPopup("ParentPopup");
		}

		if (ImGui::BeginPopup("ParentPopup")) {
			for (Vortex::Entity* entity : m_entities) {
				if (entity != m_selectedEntity) {
					if (ImGui::Selectable(entity->name.c_str())) {
						if (m_selectedEntity->GetParent() != nullptr) {
							m_selectedEntity->GetParent()->RemoveChild(entity);
						}
						m_selectedEntity->SetParent(entity);
					}
				}
			}
			ImGui::EndPopup();
		}

		ImGui::Text("Position");
		float tempX = m_selectedEntity->GetPosition().x;
		float tempY = m_selectedEntity->GetPosition().y;
		if (ImGui::DragFloat("X", &tempX, 1.0f, 1.0f)) {
			m_selectedEntity->SetPosition(tempX, m_selectedEntity->GetPosition().y);
		}
		if (ImGui::DragFloat("Y", &tempY, 1.0f, 1.0f)) {
			m_selectedEntity->SetPosition(m_selectedEntity->GetPosition().x, tempY);
		}

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
					ImGui::DragFloat("Position Offset X#lightX", &pointlight->originOffset.x);
					ImGui::DragFloat("Position Offset Y#lightY", &pointlight->originOffset.y);
				}
				Vortex::Rigidbody* rigid = dynamic_cast<Vortex::Rigidbody*>(component);
				if (rigid) {
					ImGui::Text("- Rigidbody");
					Vortex::CollisionShape colliderShape = rigid->collider->GetCollisionShapeType();
					if (ImGui::Button("Switch to Type Rectangle")) {
						if (colliderShape != Vortex::CollisionShape::Rectangle) {
							delete rigid->collider;
							rigid->Initialize(Vortex::CollisionShape::Rectangle);
						}
					}
					if (ImGui::Button("Switch to Type Circle")) {
						if (colliderShape != Vortex::CollisionShape::Circle) {
							delete rigid->collider;
							rigid->Initialize(Vortex::CollisionShape::Circle);
						}
					}
					if (ImGui::Button("Switch to Type Static Occlusion")) {
						if (!physics && sprite) {
							delete rigid->collider;
							rigid->Initialize(Vortex::CollisionShape::StaticOcclusion);
						}
						else {
							std::cout << "Cannot switch to type Static Occlusion, Physics2D present or SpriteRenderer2D absent." << std::endl;
						}
					}
					switch (colliderShape) {
						case Vortex::CollisionShape::Rectangle:
							ImGui::DragFloat("Position Offset X#colliderX", &reinterpret_cast<Vortex::RectangleCollisionContainer*>(rigid->collider)->originOffset.x);
							ImGui::DragFloat("Position Offset Y#colliderY", &reinterpret_cast<Vortex::RectangleCollisionContainer*>(rigid->collider)->originOffset.y);
							ImGui::DragFloat("Collider Width", &reinterpret_cast<Vortex::RectangleCollisionContainer*>(rigid->collider)->bounds.x);
							ImGui::DragFloat("Collider Height", &reinterpret_cast<Vortex::RectangleCollisionContainer*>(rigid->collider)->bounds.y);
							break;
						case Vortex::CollisionShape::Circle:
							ImGui::DragFloat("Position Offset X#colliderX", &reinterpret_cast<Vortex::CircleCollisionContainer*>(rigid->collider)->originOffset.x);
							ImGui::DragFloat("Position Offset Y#colliderY", &reinterpret_cast<Vortex::CircleCollisionContainer*>(rigid->collider)->originOffset.y);
							ImGui::DragFloat("Collider Radius", &reinterpret_cast<Vortex::CircleCollisionContainer*>(rigid->collider)->radius);
							break;
						case Vortex::CollisionShape::StaticOcclusion:
							Vortex::StaticOcclusionCollisionContainer* container = reinterpret_cast<Vortex::StaticOcclusionCollisionContainer*>(rigid->collider);
							ImGui::DragFloat("Position Offset X#colliderX", &container->originOffset.x);
							ImGui::DragFloat("Position Offset Y#colliderY", &container->originOffset.y);
							ImGui::Image((ImTextureID)(intptr_t)container->occlusionTexture.ID, ImVec2(container->bounds.x, container->bounds.y), ImVec2(0, 1), ImVec2(1, 0));
							break;
					}
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
			if (ImGui::Selectable("Rigidbody")) {
				Vortex::Rigidbody* rigidbodyComponent = new Vortex::Rigidbody();
				m_selectedEntity->AddComponent(rigidbodyComponent);
				rigidbodyComponent->Initialize(Vortex::CollisionShape::Rectangle);
				std::cout << "Added Rigidbody Component to " << m_selectedEntity->name << std::endl;
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