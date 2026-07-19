#pragma once

#include <filesystem>
#include <vector>
#include <map>
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
#include "Core/Entities/Tile.h"
#include "Core/Utility/Shader.h"
#include "Core/Utility/SceneUtility.h"
#include "Core/Utility/ScriptingInterface.h"
#include "Core/Utility/Collisions.h"
#include "Core/Utility/Math.h"
#include "Core/Components/SpriteRenderer2D.h"
#include "Core/Components/Physics2D.h"
#include "Core/Components/PointLight.h"
#include "Core/Components/Rigidbody.h"

enum class DisplayMode {
    Combined,
    Albedo,
    Ambient,
    Occlusion,
    BlurPass1,
    BlurPass2
};

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
    Shader* m_compositionShader = nullptr;
	Shader* m_gaussBlurShader = nullptr;
    Shader* m_lightingShader = nullptr;
    
    std::string currentScene = "Default Scene";
    bool m_isOpen = false;
	bool m_isRunning = false;
    bool m_gravity = true;
    float m_bounciness = 1.0f;
    bool m_isDragging = false;
	Vortex::Entity* m_selectedEntity = nullptr;
	Vortex::Vec2 m_dragOffset = { 0.0f, 0.0f };

    float m_meterToPixel = 20.0f;

    DisplayMode m_chosenDisplayMode;
    Vortex::Rect m_viewportRect;
    GLuint m_ssbo;
	std::vector<float> m_lightData;
    unsigned int m_viewport_fbo;
    unsigned int m_albedo_fbo;
    unsigned int m_ambient_fbo;
    unsigned int m_occlusion_fbo;
    unsigned int m_blur_fbo1;
    unsigned int m_blur_fbo2;
    unsigned int m_viewportTexture;
    unsigned int m_albedoTexture;
    unsigned int m_ambientTexture;
    unsigned int m_occlusionTexture;
    unsigned int m_blurTexture1;
    unsigned int m_blurTexture2;
    float m_blurScale = 1.0f;
    float m_shadowFalloff = 5.0f;
    unsigned int m_albedo_rbo;
    unsigned int m_occlusion_rbo;
    glm::mat4 m_currentViewMatrix;
    Vortex::Vec2 m_viewportSize = { 1280.0f, 720.0f };
	bool m_tilingMode = false;
    std::vector<std::string> m_tileLocations;
    std::string m_selectedTileLocation = "None";

    float m_gravityStrength = 9.8f;
    Vortex::Vec2 m_gravityDirection = { 0.0f, 1.0f };
    float m_magicColor[4] = { 0.5f, 0.0f, 1.0f, 1.0f };

    std::vector<Vortex::Entity*> m_entities;
    std::map<Vortex::Entity, Vortex::Vec2> m_entityPositions;
};
