#include "PointLight.h"
#include <glad/glad.h>
#include <iostream>

using namespace Vortex;

PointLight::PointLight(Shader* shader) : shader(shader) {
	initRenderData();
	if (!shader) {
		this->shader = new Shader("assets/shaders/basicVertex.glsl", "assets/shaders/lightFragment.glsl");
	}
	this->singleColorShader = new Shader("assets/shaders/basicVertex.glsl", "assets/shaders/singleColorFragment.glsl");
};

void PointLight::initRenderData() {
	unsigned int VBO;
	// Basic Quad.
	float vertices[] = {
		// pos
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Unbind Buffers.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

PointLight::~PointLight() {
	glDeleteVertexArrays(1, &this->quadVAO);
}

void PointLight::Illuminate(Vortex::Vec2 position, glm::mat4 viewMatrix) {

	// Activate Shader Program.
	this->shader->use();

	Vortex::Vec2 center = Vortex::Vec2(position.x + (owner->bounds.w / 2.0f), position.y + (owner->bounds.h / 2.0f));

	// Model Matrix (Local Space).
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(center.x - radius, center.y - radius, 0.0f));

	model = glm::scale(model, glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));

	// Set Shader Uniforms.
	this->shader->setMat4("model", model);
	glm::mat4 projection = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f, -1.0f, 50.0f);
	this->shader->setMat4("projection", projection);
	this->shader->setMat4("view", viewMatrix);
	this->shader->setVec2("center", center);
	this->shader->setFloat("radius", radius);
	this->shader->setFloat("brightness", brightness);
	this->shader->setFloat("falloff", falloff);
	this->shader->setVec3("lightColor", color);

	// Stencil Buffer Stuff
	glStencilFunc(GL_ALWAYS, 1, 0xFF); // All fragments should pass the stencil test.
	glStencilMask(0xFF); // Enable writing to the stencil buffer.

	// Bind VAO, Draw, then Unbind.
	glBindVertexArray(this->quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// If selected, draw a blue outline.
	if (owner->isSelected) {
		// Stencil Buffer Stuff
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00); // Disable writing to the stencil buffer.
		glDisable(GL_DEPTH_TEST);
		singleColorShader->use();

		// Draw the Outline.
		float thickness = 5.0f; // Thickness in pixels.
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3((glm::vec2)position, 0.0f));
		model = glm::translate(model, glm::vec3(0.5f * radius, 0.5f * radius, 0.0f)); // Move to center.

		// Offset back by half of the new (sprite + thickness) size.
		model = glm::translate(model, glm::vec3(-0.5f * (radius + thickness), -0.5f * (radius + thickness), 0.0f));

		// Scale by the absolute pixel size plus thickness.
		model = glm::scale(model, glm::vec3(radius + thickness, radius + thickness, 1.0f));


		singleColorShader->setMat4("model", model);
		glm::mat4 projection = glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f, 0.1f, 50.0f);
		singleColorShader->setMat4("projection", projection);
		this->singleColorShader->setMat4("view", viewMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0xFF);
		glEnable(GL_DEPTH_TEST);
	}
	glBindVertexArray(0);


}