#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Core/Entity.h"
#include "Core/Shader.h"
#include "Core/Textures.h"

class SpriteRenderer {
public:
	SpriteRenderer(Shader &shader);
	~SpriteRenderer();

	void DrawSprite(Vortex::Texture2D& texture, Vortex::Vec2 position, Vortex::Vec2 size, float rotation, Vortex::Vec3 color);

private:
	Shader shader;
	unsigned int quadVAO;

	void initRenderData();
};