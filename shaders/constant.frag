#version 330 core

// Constant (ambient) model - only the ambient term, independent of the light and the normal.

in vec3 worldPosition;
in vec3 worldNormal;
in vec3 vColor;

uniform vec3 ambientLight = vec3(1.0);

out vec4 fragColor;

void main()
{
    fragColor = vec4(ambientLight * vColor, 1.0);
}
