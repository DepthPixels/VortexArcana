#pragma once
#include <vector>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "Core/Entity.h"

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
    SDL_Renderer* m_renderer = nullptr;
    bool m_isOpen = false;
	bool m_isRunning = false;
    bool m_isDragging = false;
	Vortex::Entity* m_selectedEntity = nullptr;
	Vortex::Vec2 m_dragOffset = { 0.0f, 0.0f };

    float m_meterToPixel = 20.0f;

    Vortex::Rect m_viewportRect;

    SDL_Texture* m_gameTexture = nullptr;
    Vortex::Vec2 m_viewportSize = { 1280, 720 };

    float m_gravityStrength = 9.8f;
    Vortex::Vec2 m_gravityDirection = { 0.0f, 1.0f };
    float m_magicColor[4] = { 0.5f, 0.0f, 1.0f, 1.0f };

    std::vector<Vortex::Entity> m_entities;
};
