#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;   // barva vrcholu zaroven urcuje, o jakou cast hodin jde

uniform mat4 modelMatrix = mat4(1.0);
uniform mat4 viewMatrix = mat4(1.0);
uniform mat4 projectionMatrix = mat4(1.0);

// Cas ve vterinach od pulnoci (nastavuje aplikace).
uniform float clockSeconds = 0.0;

out vec3 vColor;

const float TWO_PI = 6.28318530718;

// Rotace bodu kolem osy Z (stred hodin je v pocatku modelu).
vec3 rotateZ(vec3 p, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return vec3(p.x * c - p.y * s, p.x * s + p.y * c, p.z);
}

bool isColor(vec3 a, vec3 b)
{
    return all(lessThan(abs(a - b), vec3(0.1)));
}

void main()
{
    vec3 p = position;
    vec3 outColor = color;

    // Rozliseni casti hodin podle barvy vrcholu:
    //   cervena = vterinova rucicka, zelena = minutova rucicka, modra = hodinova rucicka,
    //   ostatni (cerna, bila, zluta, smetanova) se neotaci.
    // Hodiny se divame zepredu (z -Z, osa X je na obrazovce vlevo), kladna rotace kolem +Z
    // proto jde po smeru hodinovych rucicek.
    if (isColor(color, vec3(1.0, 0.0, 0.0)))
    {
        p = rotateZ(p, TWO_PI * mod(clockSeconds, 60.0) / 60.0);
        outColor = vec3(0.85, 0.1, 0.1);
    }
    else if (isColor(color, vec3(0.0, 1.0, 0.0)))
    {
        p = rotateZ(p, TWO_PI * mod(clockSeconds, 3600.0) / 3600.0);
        outColor = vec3(0.08);
    }
    else if (isColor(color, vec3(0.0, 0.0, 1.0)))
    {
        p = rotateZ(p, TWO_PI * mod(clockSeconds, 43200.0) / 43200.0);
        outColor = vec3(0.08);
    }

    vColor = outColor;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(p, 1.0);
}
