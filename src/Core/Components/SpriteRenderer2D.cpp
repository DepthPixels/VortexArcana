#include "SpriteRenderer2D.h"
#include <glad/glad.h>

using namespace Vortex;

SpriteRenderer2D::SpriteRenderer2D(Shader& shader)
	: shader("assets/shaders/basicVertex.glsl", "assets/shaders/basicFragment.glsl") {
	initRenderData();
	this->shader = shader;
};
	
void SpriteRenderer2D::initRenderData() {
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

SpriteRenderer2D::~SpriteRenderer2D() {
	glDeleteVertexArrays(1, &this->quadVAO);
}

void SpriteRenderer2D::LoadSprite(const char* location, bool alpha) {
	int width, height, nrChannels;
	unsigned char* data = stbi_load(location, &width, &height, &nrChannels, 4);
	if (data)
	{
		if (alpha) {
			this->texture.internalFormat = GL_RGBA;
			this->texture.imageFormat = GL_RGBA;
		}
		this->texture.Generate(width, height, data);
		stbi_image_free(data);
		this->spriteAssigned = true;
	}
}

void SpriteRenderer2D::DrawSprite(Vortex::Vec2 position, Vortex::Vec2 size, float rotation, Vortex::Vec3 color) {

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
	this->texture.Bind();

	// Bind VAO, Draw, then Unbind.
	glBindVertexArray(this->quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}