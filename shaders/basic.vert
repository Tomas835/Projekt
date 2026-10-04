#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform vec3 offset;
uniform float scale;
uniform float rotation;

out vec3 vColor;

void main()
{
    // Zakladni transformace bez matic:
    // x' = s * (x*cos(a) - y*sin(a)) + tx
    // y' = s * (x*sin(a) + y*cos(a)) + ty
    // z' = s*z + tz
    float c = cos(rotation);
    float s = sin(rotation);

    float x = position.x * c - position.y * s;
    float y = position.x * s + position.y * c;

    vec3 transformed = vec3(x, y, position.z) * scale + offset;

    vColor = color;
    gl_Position = vec4(transformed, 1.0);
}
