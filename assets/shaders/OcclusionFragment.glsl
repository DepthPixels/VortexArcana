#version 460 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture1;

vec4 pixelColor;

void main() {
	pixelColor = texture(texture1, TexCoord);

	if (pixelColor.w > 0.0f) {
		FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	} else {
		FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}