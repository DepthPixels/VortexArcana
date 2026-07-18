#version 460 core
in vec2 Pos; // From Vertex Shader
out vec4 FragColor;

layout(std430, binding = 0) buffer LightData {
    vec2 lightPositions[];
};
uniform int lightCount;

void main()
{
    // Start with a dark background to make lights pop
    vec3 color = vec3(0.05, 0.05, 0.05);

    // Iterate and draw circles for every light position
    for (int i = 0; i < lightCount; i++) {
        float dist = distance(Pos, lightPositions[i]);
        
        // Draw a white circle with a 50px radius
        if (dist < 50.0) {
            color = vec3(1.0, 1.0, 1.0);
        }
    }

    FragColor = vec4(color, 1.0);
}