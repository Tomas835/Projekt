#version 330 core

// Phong WITHOUT clipping the highlight on the far side:
// ambient + diffuse + specular term (reflected vector R = reflect(-L, N)).

in vec3 worldPosition;
in vec3 worldNormal;
in vec3 vColor;

uniform vec3 lightPosition = vec3(0.0);
uniform vec3 lightColor = vec3(1.0);
uniform vec3 viewPosition = vec3(0.0);
uniform vec3 ambientLight = vec3(0.1);
uniform float specularStrength = 0.8;
uniform float shininess = 32.0;

out vec4 fragColor;

void main()
{
    vec3 N = normalize(worldNormal);
    vec3 L = normalize(lightPosition - worldPosition);
    vec3 V = normalize(viewPosition - worldPosition);

    float diff = max(dot(N, L), 0.0);

    // BUG: the specular term is also computed on the far side (diff <= 0),
    // so the highlight "leaks" onto the side facing away from the light.
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), shininess);

    vec3 ambient = ambientLight * vColor;
    vec3 diffuse = diff * lightColor * vColor;
    vec3 specular = specularStrength * spec * lightColor;

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
