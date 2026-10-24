#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;    // material (vertex) color
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
    // normal matrix - correct even for non-uniform scaling
    worldNormal = transpose(inverse(mat3(modelMatrix))) * normal;
    vColor = color;
    gl_Position = projectionMatrix * viewMatrix * world;
}
