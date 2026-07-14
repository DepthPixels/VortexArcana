#version 460 core
layout (location = 0) in vec2 aPos;
out vec2 Pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	vec4 worldSpace = model * vec4(aPos, 0.0, 1.0);
	Pos = worldSpace.xy;
	gl_Position = projection * view * worldSpace;
}