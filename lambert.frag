#version 330 core

// Lambert - ambient + diffuse term (Lambert's law: cosine of the angle between N and L).

in vec3 worldPosition;
in vec3 worldNormal;
in vec3 vColor;

uniform vec3 lightPosition = vec3(0.0);
uniform vec3 lightColor = vec3(1.0);
uniform vec3 ambientLight = vec3(0.1);

out vec4 fragColor;

void main()
{
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(lightPosition - worldPosition);

    float diff = max(dot(N, L), 0.0);

    vec3 ambient = ambientLight * vColor;
    vec3 diffuse = diff * lightColor * vColor;

    fragColor = vec4(ambient + diffuse, 1.0);
}
