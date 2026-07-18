#include "PointLight.h"
#include <glad/glad.h>
#include <iostream>

using namespace Vortex;

PointLight::~PointLight() {}

void PointLight::Illuminate(Vortex::Vec2 position, std::vector<float>& lightData, int& lightIndex) {
	lightData.push_back(position.x);
	lightData.push_back(position.y);
	lightData.push_back(radius);
	lightData.push_back(brightness);
	lightData.push_back(falloff);
	lightData.push_back(color.x);
	lightData.push_back(color.y);
	lightData.push_back(color.z);
	lightIndex += 1;
}