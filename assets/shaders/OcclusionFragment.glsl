#version 460 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture1;

vec4 pixelColor;

void main() {
	pixelColor = texture(texture1, TexCoord);

	if (pixelColor.a < 0.1) {
		discard;
	}

	FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}