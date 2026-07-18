#include "Textures.h"
#include <glad/glad.h>

using namespace Vortex;

Texture2D::Texture2D()
	: width(0), height(0),
	internalFormat(GL_RGBA), imageFormat(GL_RGBA),
	wrapS(GL_CLAMP_TO_EDGE), wrapT(GL_CLAMP_TO_EDGE),
	filterMin(GL_NEAREST), filterMax(GL_NEAREST) {
	// Constructor Defaults.
}

void Texture2D::Generate(unsigned int width, unsigned int height, unsigned char* data) {
	this->width = width;
	this->height = height;

	// Generate Texture at ID.
	glGenTextures(1, &this->ID);
	// Bind Texture.
	glBindTexture(GL_TEXTURE_2D, this->ID);
	// Load Texture Data.
	glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, width, height, 0, this->imageFormat, GL_UNSIGNED_BYTE, data);
	// Set Texture Parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->filterMin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->filterMax);
	// Unbind Texture.
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Bind() const {
	glBindTexture(GL_TEXTURE_2D, this->ID);
}