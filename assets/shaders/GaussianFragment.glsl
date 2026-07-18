#version 460 core

out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D u_image;
uniform vec2 u_dir; // (1.0, 0.0) for horizontal, (0.0, 1.0) for vertical
uniform float u_radius; // The blur radius in pixels
uniform float u_blurScale;

void main() {
    // 9-tap Gaussian weights
    float weight[3];
    weight[0] = 0.44198;
    weight[1] = 0.25918;
    weight[2] = 0.05399;

    // Center pixel
    vec3 result = texture2D(u_image, texCoords).rgb * weight[0];
    
    // Accumulate neighboring pixels
    for (int i = 1; i <= 2; i++) {
        float offset = float(i) * 1.33333333; // Magic number
        vec2 texOffset = u_dir * (offset * u_blurScale) / u_radius;
        
        result += texture2D(u_image, texCoords + texOffset).rgb * weight[i];
        result += texture2D(u_image, texCoords - texOffset).rgb * weight[i];
    }

    FragColor = vec4(result, 1.0);
}