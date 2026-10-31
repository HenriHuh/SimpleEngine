#version 330 core

// Attribute locations match the position, normal, and UV layout configured by Mesh.
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTextureCoordinates;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

out vec3 vWorldNormal;

void main()
{
    // Normals need the inverse-transpose of the model transform to remain
    // perpendicular to a surface when an object is scaled.
    vWorldNormal = normalize(uNormalMatrix * aNormal);

    // Move the vertex from local space, through world and camera space, into
    // clip space. OpenGL uses gl_Position to assemble and rasterize triangles.
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
