#version 330 core

in vec3 vWorldNormal;

out vec4 fragColor;

uniform vec3 uColor;
uniform vec3 uLightDirection;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;

void main()
{
    vec3 normal = normalize(vWorldNormal);
    float diffuseStrength = max(dot(normal, normalize(uLightDirection)), 0.0);
    vec3 lighting = uAmbientColor + uLightColor * diffuseStrength;
    fragColor = vec4(uColor * lighting, 1.0);
}
