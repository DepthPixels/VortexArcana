#version 460 core

out vec4 FragColor;

// in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec3 spriteColor;

void main() {
	FragColor = texture(texture1, TexCoord) * vec4(spriteColor, 1.0f);
}