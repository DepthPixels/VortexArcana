#pragma once

#include <vector>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Core/Entity.h"
#include "Core/Utility/Shader.h"
#include "Core/Components/SpriteRenderer2D.h"
#include "Core/Components/Physics2D.h"

class Engine {
public:
    Engine();
    ~Engine();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    void ProcessInput();
    void Update(float deltaTime);
    void ShowViewportWindow();
    void ShowEditorUI();
    void Render();

    // Helper Functions
    Vortex::Vec2 CoordsScreenToViewport(Vortex::Vec2 screenCoords);
	Vortex::Entity* GetEntityAtViewportCoords(Vortex::Vec2 viewportCoords);

    SDL_Window* m_window = nullptr;

	SDL_GLContext m_glContext = nullptr;
    unsigned int m_vao, m_vbo, m_ebo;
    
    // Shaders
    unsigned int m_shaderProgram;
    Shader* m_basicShader = nullptr;

    bool m_isOpen = false;
	bool m_isRunning = false;
    bool m_isDragging = false;
	Vortex::Entity* m_selectedEntity = nullptr;
	Vortex::Vec2 m_dragOffset = { 0.0f, 0.0f };

    float m_meterToPixel = 20.0f;

    Vortex::Rect m_viewportRect;
    unsigned int m_fbo;
    unsigned int m_viewportTexture;
    unsigned int m_rbo;
    Vortex::Vec2 m_viewportSize = { 1280.0f, 720.0f };

    float m_gravityStrength = 9.8f;
    Vortex::Vec2 m_gravityDirection = { 0.0f, 1.0f };
    float m_magicColor[4] = { 0.5f, 0.0f, 1.0f, 1.0f };

    std::vector<Vortex::Entity*> m_entities;
};
