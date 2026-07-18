#version 460 core
in vec2 Pos;
out vec4 FragColor;

uniform vec2 center;
uniform float radius;
uniform float brightness;
uniform float falloff;
uniform vec3 lightColor;

layout(binding = 5) uniform sampler2D occlusionMap; 

// Return Value: 1 is lit, 0 otherwise.
int rayMarch(vec2 rayOrigin, float rayAngle) {
    float rayProgress = 0.0f;
    vec2 originalCoords = vec2(rayOrigin.x/1280.0f, 1.0f - (rayOrigin.y/720.0f));
    vec4 initialCheck = texture(occlusionMap, originalCoords);
    bool inMap = initialCheck.x > 0.5f;
    if (inMap) {
        return 1;
    }
    for (rayProgress = 0.0f; rayProgress < 1000; rayProgress++) {
        if (rayProgress > distance(rayOrigin, center)) {
            return 1;
        }
        vec2 occlusionTexCoords = vec2((rayOrigin.x + (rayProgress * cos(rayAngle)))/1280.0f, 1.0f - ((rayOrigin.y + (rayProgress * sin(rayAngle)))/720.0f));
        vec4 occlusionCheck = texture(occlusionMap, occlusionTexCoords);
        if (occlusionCheck.x > 0.5f) {
            return 0;
        }
    }
    return 1;
}

void main()
{
    float angle = atan((center.y - Pos.y), (center.x - Pos.x));
    int result = rayMarch(Pos, angle);

    float dist = distance(Pos, center);

    float attenuation = max(0.0, 1.0 - (dist/radius));
    attenuation = pow(attenuation, falloff);

    if (result == 1) {
        FragColor = vec4(lightColor * brightness * attenuation, 1.0);
    } else {
        FragColor = vec4(0.0f, 0.0f, 0.0f, 0.5f);
    }
}