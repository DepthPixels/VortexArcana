#include "SpriteRenderer.h"
#include <glad/glad.h>

SpriteRenderer::SpriteRenderer(Shader& shader)
	: shader("assets/shaders/basicVertex.glsl", "assets/shaders/basicFragment.glsl") {
	initRenderData();
	this->shader = shader;
};
	
void SpriteRenderer::initRenderData() {
	unsigned int VBO;
	// Basic Quad.
	float vertices[] = {
		// pos      // tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	// Generate VAO and VBO.
	glGenVertexArrays(1, &this->quadVAO);
	glGenBuffers(1, &VBO);

	// Bind VBO and load vertex data.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind VAO.
	glBindVertexArray(this->quadVAO);

	// Load Data into location 0.
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Unbind Buffers.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

SpriteRenderer::~SpriteRenderer() {
	glDeleteVertexArrays(1, &this->quadVAO);
}

void SpriteRenderer::DrawSprite(Vortex::Texture2D& texture, Vortex::Vec2 position, Vortex::Vec2 size, float rotation, Vortex::Vec3 color) {

	// Activate Shader Program.
	this->shader.use();

	// Model Matrix (Local Space).
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3((glm::vec2)position, 0.0f));

	model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
	model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

	model = glm::scale(model, glm::vec3((glm::vec2)size, 1.0f));

	// Set Shader Uniforms.
	shader.setMat4("model", model);
	shader.setVec3("spriteColor", color);

	// Bind Texture.
	glActiveTexture(GL_TEXTURE0);
	texture.Bind();

	// Bind VAO, Draw, then Unbind.
	glBindVertexArray(this->quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}