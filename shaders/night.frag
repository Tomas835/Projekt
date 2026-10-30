#version 330 core

// Night scene: Phong lighting (Ia + Id + Is) with ONE point light (firefly / lantern).
// The light is attenuated with the distance d from the source:
//     attenuation = 1 / (kc + kl * d + kq * d^2)
// so only the close surroundings of the light are lit; the rest of the forest stays
// almost dark (only a very weak ambient term + distance fog).

in vec3 worldPosition;
in vec3 worldNormal;
in vec3 vColor;

uniform vec3 lightPosition = vec3(0.0);
uniform vec3 lightColor = vec3(1.0);
uniform vec3 lightAttenuation = vec3(1.0, 0.0, 0.0);   // kc, kl, kq
uniform vec3 viewPosition = vec3(0.0);
uniform vec3 ambientLight = vec3(0.012, 0.016, 0.035);  // Ia: very weak bluish night ambient
uniform float specularStrength = 0.05;                  // material: Is strength (how shiny the surface is)
uniform float shininess = 8.0;                          // material: size of the highlight
uniform vec3 fogColor = vec3(0.004, 0.006, 0.012);      // must match the night clear color
uniform float fogDensity = 0.045;                       // distant parts fade into darkness

out vec4 fragColor;

void main()
{
    vec3 N = normalize(worldNormal);
    vec3 toLight = lightPosition - worldPosition;
    float dist = length(toLight);
    vec3 L = toLight / dist;
    vec3 V = normalize(viewPosition - worldPosition);

    // the light fades quickly with the distance from the source
    float attenuation = 1.0 / (lightAttenuation.x + lightAttenuation.y * dist + lightAttenuation.z * dist * dist);

    float diff = max(dot(N, L), 0.0);                       // Id: diffuse term
    float spec = 0.0;
    if (diff > 0.0)   // clip the highlight on the side facing away from the light
    {
        vec3 R = reflect(-L, N);
        spec = pow(max(dot(V, R), 0.0), shininess);         // Is: specular term
    }

    vec3 ambient = ambientLight * vColor;                   // Ia: not attenuated (weak moon/sky light)
    vec3 diffuse = diff * lightColor * vColor;
    vec3 specular = specularStrength * spec * lightColor;

    vec3 color = ambient + attenuation * (diffuse + specular);

    // exponential fog: far parts of the forest blend into the dark background
    float viewDist = length(viewPosition - worldPosition);
    float fog = exp(-pow(fogDensity * viewDist, 2.0));
    fragColor = vec4(mix(fogColor, color, fog), 1.0);
}
