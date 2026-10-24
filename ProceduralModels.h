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


    // ---------------------------------------------------------------
    // Models for lighting: position (3), color (3), normal (3)
    // ---------------------------------------------------------------
    inline void addLitVertex(Data& d, const glm::vec3& p, const glm::vec3& color, const glm::vec3& n)
    {
        d.insert(d.end(), { p.x, p.y, p.z, color.r, color.g, color.b, n.x, n.y, n.z });
    }

    // Flat-shaded triangle (one normal for the whole face).
    inline void addLitFlat(Data& d, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
        const glm::vec3& color)
    {
        const glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
        addLitVertex(d, a, color, n);
        addLitVertex(d, b, color, n);
        addLitVertex(d, c, color, n);
    }

    // Circular cap (disk) around the Y axis, normal up (up = +1) or down (up = -1).
    inline void addLitDisk(Data& d, float y, float radius, int seg, const glm::vec3& color, float up)
    {
        for (int i = 0; i < seg; ++i)
        {
            const float a0 = 2.0f * PI * i / seg, a1 = 2.0f * PI * (i + 1) / seg;
            const glm::vec3 n(0, up, 0);
            addLitVertex(d, { 0, y, 0 }, color, n);
            addLitVertex(d, { radius * std::cos(a0), y, radius * std::sin(a0) }, color, n);
            addLitVertex(d, { radius * std::cos(a1), y, radius * std::sin(a1) }, color, n);
        }
    }

    // Cylinder / truncated cone (Y axis) with smooth normals: radius r0 at y0, radius r1 at y1.
    inline void addLitFrustum(Data& d, float y0, float r0, float y1, float r1, int seg, const glm::vec3& color)
    {
        const float h = y1 - y0, dr = r0 - r1;
        for (int i = 0; i < seg; ++i)
        {
            const float a0 = 2.0f * PI * i / seg, a1 = 2.0f * PI * (i + 1) / seg;
            const glm::vec3 d0(std::cos(a0), 0, std::sin(a0)), d1(std::cos(a1), 0, std::sin(a1));
            // side normal: (h * direction, dr) - for a cylinder (dr = 0) it points straight outwards
            const glm::vec3 n0 = glm::normalize(glm::vec3(d0.x * h, dr, d0.z * h));
            const glm::vec3 n1 = glm::normalize(glm::vec3(d1.x * h, dr, d1.z * h));
            const glm::vec3 b0 = glm::vec3(r0 * d0.x, y0, r0 * d0.z), b1 = glm::vec3(r0 * d1.x, y0, r0 * d1.z);
            const glm::vec3 t0 = glm::vec3(r1 * d0.x, y1, r1 * d0.z), t1 = glm::vec3(r1 * d1.x, y1, r1 * d1.z);
            addLitVertex(d, b0, color, n0); addLitVertex(d, b1, color, n1); addLitVertex(d, t1, color, n1);
            addLitVertex(d, b0, color, n0); addLitVertex(d, t1, color, n1); addLitVertex(d, t0, color, n0);
        }
    }

    // Rocket: the nose points to +Y, spans approximately y = -0.8 to y = 1.15.
    inline Data rocket()
    {
        Data d;
        const int seg = 24;
        const glm::vec3 hull(0.90f, 0.90f, 0.93f);
        const glm::vec3 red(0.85f, 0.12f, 0.10f);
        const glm::vec3 grey(0.35f, 0.35f, 0.38f);
        const glm::vec3 blue(0.20f, 0.45f, 0.85f);

        addLitFrustum(d, -0.60f, 0.22f, 0.50f, 0.22f, seg, hull);   // hull
        addLitFrustum(d, 0.50f, 0.22f, 1.15f, 0.0f, seg, red);      // nose cone
        addLitFrustum(d, -0.15f, 0.225f, 0.05f, 0.225f, seg, blue); // stripe
        addLitFrustum(d, -0.78f, 0.17f, -0.60f, 0.12f, seg, grey);  // nozzle
        addLitDisk(d, -0.78f, 0.17f, seg, grey, -1.0f);
        addLitDisk(d, -0.60f, 0.22f, seg, hull, -1.0f);

        // 3 stabilizing fins (double-sided, slightly thick)
        for (int f = 0; f < 3; ++f)
        {
            const float a = 2.0f * PI * f / 3.0f;
            const glm::vec3 axisX(std::cos(a), 0, std::sin(a));
            const glm::vec3 axisZ(-std::sin(a), 0, std::cos(a));
            const float t = 0.012f;
            auto P = [&](float r, float y, float side) { return axisX * r + glm::vec3(0, y, 0) + axisZ * side; };
            for (float side : { t, -t })
            {
                const glm::vec3 p1 = P(0.20f, 0.00f, side), p2 = P(0.55f, -0.80f, side), p3 = P(0.20f, -0.80f, side);
                if (side > 0) addLitFlat(d, p1, p3, p2, red);   // vertex order determines the normal direction
                else          addLitFlat(d, p1, p2, p3, red);
            }
        }
        return d;
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
