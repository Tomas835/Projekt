#version 330 core

// WRONG conversion of the normal from local to world space:
// the normal is multiplied directly by the model matrix. With non-uniform scaling
// the normal stops being perpendicular to the surface, so the lighting is wrong.

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;

uniform mat4 modelMatrix = mat4(1.0);
uniform mat4 viewMatrix = mat4(1.0);
uniform mat4 projectionMatrix = mat4(1.0);

out vec3 worldPosition;
out vec3 worldNormal;
out vec3 vColor;

void main()
{
    vec4 world = modelMatrix * vec4(position, 1.0);
    worldPosition = world.xyz;
    worldNormal = mat3(modelMatrix) * normal;          // <-- BUG
    vColor = color;
    gl_Position = projectionMatrix * viewMatrix * world;
}
