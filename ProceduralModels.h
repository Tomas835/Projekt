#pragma once
#include <glm/glm.hpp>
#include <cmath>
#include <random>
#include <vector>

namespace proc
{
    using Data = std::vector<float>;
    constexpr float PI = 3.14159265358979323846f;


    inline void addTri(Data& d, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
        const glm::vec3& color, const glm::vec3& inside)
    {
        glm::vec3 n = glm::cross(b - a, c - a);
        const float len = glm::length(n);
        n = (len > 1e-8f) ? n / len : glm::vec3(0, 1, 0);
        const glm::vec3 centroid = (a + b + c) / 3.0f;
        if (glm::dot(n, centroid - inside) < 0.0f) n = -n;

        const glm::vec3 light = glm::normalize(glm::vec3(0.4f, 0.8f, 0.5f));
        const float k = 0.55f + 0.45f * std::max(0.0f, glm::dot(n, light));
        const glm::vec3 col = glm::clamp(color * k, 0.0f, 1.0f);

        for (const glm::vec3& p : { a, b, c })
        {
            d.insert(d.end(), { p.x, p.y, p.z, col.r, col.g, col.b });
        }
    }

    inline void addCylinder(Data& d, const glm::vec3& base, float radius, float height, int seg,
        const glm::vec3& color)
    {
        const glm::vec3 inside = base + glm::vec3(0, height * 0.5f, 0);
        const glm::vec3 top = base + glm::vec3(0, height, 0);
        for (int i = 0; i < seg; ++i)
        {
            const float a0 = 2.0f * PI * i / seg, a1 = 2.0f * PI * (i + 1) / seg;
            const glm::vec3 p0(radius * std::cos(a0), 0, radius * std::sin(a0));
            const glm::vec3 p1(radius * std::cos(a1), 0, radius * std::sin(a1));
            addTri(d, base + p0, base + p1, top + p1, color, inside);
            addTri(d, base + p0, top + p1, top + p0, color, inside);
            addTri(d, top, top + p0, top + p1, color, inside);
        }
    }

    inline void addCone(Data& d, const glm::vec3& base, float radius, float height, int seg,
        const glm::vec3& color)
    {
        const glm::vec3 apex = base + glm::vec3(0, height, 0);
        const glm::vec3 inside = base + glm::vec3(0, height * 0.3f, 0);
        for (int i = 0; i < seg; ++i)
        {
            const float a0 = 2.0f * PI * i / seg, a1 = 2.0f * PI * (i + 1) / seg;
            const glm::vec3 p0(radius * std::cos(a0), 0, radius * std::sin(a0));
            const glm::vec3 p1(radius * std::cos(a1), 0, radius * std::sin(a1));
            addTri(d, base + p0, base + p1, apex, color, inside);
            addTri(d, base, base + p1, base + p0, color, inside);
        }
    }

    inline void addEllipsoid(Data& d, const glm::vec3& center, const glm::vec3& radii, int rings, int seg,
        const glm::vec3& color)
    {
        auto point = [&](int r, int s) {
            const float theta = PI * r / rings;
            const float phi = 2.0f * PI * s / seg;
            return center + glm::vec3(radii.x * std::sin(theta) * std::cos(phi),
                radii.y * std::cos(theta),
                radii.z * std::sin(theta) * std::sin(phi));
            };
        for (int r = 0; r < rings; ++r)
            for (int s = 0; s < seg; ++s)
            {
                const glm::vec3 p00 = point(r, s), p01 = point(r, s + 1);
                const glm::vec3 p10 = point(r + 1, s), p11 = point(r + 1, s + 1);
                addTri(d, p00, p10, p11, color, center);
                addTri(d, p00, p11, p01, color, center);
            }
    }

    inline const glm::vec3 TRUNK(0.40f, 0.26f, 0.12f);


    inline Data conifer(const glm::vec3& leaves)
    {
        Data d;
        addCylinder(d, glm::vec3(0, 0, 0), 0.12f, 0.9f, 6, TRUNK);
        addCone(d, glm::vec3(0, 0.6f, 0), 0.9f, 1.5f, 6, leaves);
        addCone(d, glm::vec3(0, 1.5f, 0), 0.65f, 1.3f, 6, leaves * 1.12f);
        return d;
    }

    inline Data broadleaf(const glm::vec3& leaves)
    {
        Data d;
        addCylinder(d, glm::vec3(0, 0, 0), 0.14f, 1.2f, 6, TRUNK);
        addEllipsoid(d, glm::vec3(0, 1.8f, 0), glm::vec3(0.9f, 0.8f, 0.9f), 4, 7, leaves);
        return d;
    }

    inline Data bush(const glm::vec3& leaves)
    {
        Data d;
        addEllipsoid(d, glm::vec3(0, 0.3f, 0), glm::vec3(0.55f, 0.4f, 0.55f), 3, 6, leaves);
        return d;
    }

    inline Data post()
    {
        Data d;
        addCylinder(d, glm::vec3(0, 0, 0), 0.06f, 1.0f, 8, glm::vec3(0.32f, 0.2f, 0.1f));
        return d;
    }

    inline Data ground(int n, unsigned seed)
    {
        Data d;
        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> u(0.0f, 1.0f);
        const float step = 2.0f / n;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
            {
                const float x0 = -1.0f + i * step, x1 = x0 + step;
                const float z0 = -1.0f + j * step, z1 = z0 + step;
                const float t = u(rng);
                glm::vec3 col = glm::mix(glm::vec3(0.30f, 0.21f, 0.11f), glm::vec3(0.22f, 0.30f, 0.12f), t * t);
                col *= 0.85f + 0.3f * u(rng);
                const glm::vec3 up(0, -1, 0); 
                addTri(d, { x0, 0, z0 }, { x0, 0, z1 }, { x1, 0, z1 }, col, up);
                addTri(d, { x0, 0, z0 }, { x1, 0, z1 }, { x1, 0, z0 }, col, up);
            }
        return d;
    }
}
