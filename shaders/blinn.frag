#version 330 core

// Blinn-Phong - ambient + diffuse + specular computed with the half-vector H = normalize(L + V).

in vec3 worldPosition;
in vec3 worldNormal;
in vec3 vColor;

uniform vec3 lightPosition = vec3(0.0);
uniform vec3 lightColor = vec3(1.0);
uniform vec3 viewPosition = vec3(0.0);
uniform vec3 ambientLight = vec3(0.1);
uniform float specularStrength = 0.8;
// The N-H angle is roughly half of the V-R angle, so cos(N,H) falls off more slowly,
// hence a larger exponent is needed for a highlight of similar size as in Phong.
uniform float shininess = 128.0;

out vec4 fragColor;

void main()
{
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(lightPosition - worldPosition);
    vec3 V = normalize(viewPosition - worldPosition);

    float diff = max(dot(N, L), 0.0);

    float spec = 0.0;
    if (diff > 0.0)
    {
        vec3 H = normalize(L + V);
        spec = pow(max(dot(N, H), 0.0), shininess);
    }

    vec3 ambient = ambientLight * vColor;
    vec3 diffuse = diff * lightColor * vColor;
    vec3 specular = specularStrength * spec * lightColor;

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
