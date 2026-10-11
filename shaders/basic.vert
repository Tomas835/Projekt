#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

// Modelova matice (posun, rotace, meritko a jejich kombinace) z aplikace.
uniform mat4 modelMatrix;

out vec3 vColor;

void main()
{
    vColor = color;
    // Homogenni souradnice: bod (x, y, z, 1) vynasobime modelovou matici.
    gl_Position = modelMatrix * vec4(position, 1.0);
}
