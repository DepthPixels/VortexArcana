#version 460 core
in vec2 Pos;
out vec4 FragColor;

uniform vec2 center;
//uniform float radius;
//uniform float brightness;
//uniform float falloff;
uniform vec3 lightColor;

layout(binding = 5) uniform sampler2D occlusionMap; 

int rayMarch(vec2 rayOrigin, float rayAngle) {
    float rayProgress = 0.0f;
    for (int i = 0; i < 1000; i++) {
        if (rayProgress > distance(rayOrigin, center)) {
            return 1;
        }
        vec2 occlusionTexCoords = vec2((rayOrigin.x + (rayProgress * cos(rayAngle)))/1280.0f, 1.0f - ((rayOrigin.y + (rayProgress * sin(rayAngle)))/720.0f));
        vec4 occlusionCheck = texture(occlusionMap, occlusionTexCoords);
        if (occlusionCheck.x > 0.5f) {
            return 0;
        }
        rayProgress += 1.0f;
    }
    return 1;
}

void main()
{
    float angle = atan((center.y - Pos.y), (center.x - Pos.x));
    int result = rayMarch(Pos, angle);

    if (result == 1) {
        FragColor = vec4(lightColor, 1.0f);
    } else {
        FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}