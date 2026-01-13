#pragma once
#include <vector>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
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

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool m_isOpen = false;
	bool m_isRunning = false;

    float m_meterToPixel = 20.0f;

    Vortex::Vec2 m_viewportPosition;

    SDL_Texture* m_gameTexture = nullptr;
    Vortex::Vec2 m_viewportSize = { 1280, 720 };

    float m_gravityStrength = 9.8f;
    Vortex::Vec2 m_gravityDirection = { 0.0f, 1.0f };
    float m_magicColor[4] = { 0.5f, 0.0f, 1.0f, 1.0f };

    std::vector<Vortex::Entity> m_entities;
};
