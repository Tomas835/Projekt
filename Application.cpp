#include "Application.h"
#include "models/clock.h"
#include "models/earth.h"
#include "models/moon.h"
#include "models/sphere.h"
#include "models/sun.h"
#include "models/vyl0034_logo.h"
#include "ProceduralModels.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <vector>

namespace
{
    constexpr float PI = 3.14159265358979323846f;

    Model* makeModel(std::vector<float> data)
    {
        Model* model = new Model(data.data(), static_cast<int>(data.size()));
        return model;
    }

    // Procedural model generated with normals (position, color, normal) and pure material colors
    template <class Builder>
    Model* makeLitModel(Builder build)
    {
        proc::litMode() = true;                  // addTri() emits 9 floats per vertex while this is on
        const proc::Data data = build();
        proc::litMode() = false;
        return new Model(data.data(), static_cast<int>(data.size()), 9, 3, 6);
    }

    void addTriangle(std::vector<float>& data,
        float x1, float y1, float z1,
        float x2, float y2, float z2,
        float x3, float y3, float z3,
        float r, float g, float b)
    {
        const float v[] = {
            x1,y1,z1,r,g,b, x2,y2,z2,r,g,b, x3,y3,z3,r,g,b
        };
        data.insert(data.end(), std::begin(v), std::end(v));
    }

    Model* createTriangleModel()
    {
        std::vector<float> d;
        addTriangle(d, 0.0f, 0.55f, 0.0f, -0.5f, -0.45f, 0.0f, 0.5f, -0.45f, 0.0f,
            0.95f, 0.25f, 0.10f);
        return makeModel(d);
    }

    TransformPtr place(float x, float y, float z, float scale, float tilt = 0.0f)
    {
        return makeComposite({
            std::make_shared<Translate>(glm::vec3(x, y, z)),
            std::make_shared<Rotate>(tilt, glm::vec3(0, 0, 1)),
            std::make_shared<Scale>(scale)
            });
    }

    TransformPtr placeY(float x, float y, float z, float scale, float yaw)
    {
        return makeComposite({
            std::make_shared<Translate>(glm::vec3(x, y, z)),
            std::make_shared<Rotate>(yaw, glm::vec3(0, 1, 0)),
            std::make_shared<Scale>(scale)
            });
    }


    Model* createClockModel()
    {
        std::vector<float> data(clockModel, clockModel + sizeof(clockModel) / sizeof(float));
        for (size_t i = 0; i + 8 < data.size(); i += 9)
        {
            data[i] = -data[i];
            data[i + 3] = -data[i + 3];
        }

        const int seg = 48;
        const float radius = 0.93f, z = -0.012f;
        const float color[3] = { 0.95f, 0.92f, 0.80f };
        auto addVertex = [&](float x, float y) {
            data.insert(data.end(), { x, y, z, 0.0f, 0.0f, -1.0f, color[0], color[1], color[2] });
            };
        for (int i = 0; i < seg; ++i)
        {
            const float a0 = 2.0f * PI * i / seg, a1 = 2.0f * PI * (i + 1) / seg;
            addVertex(0.0f, 0.0f);
            addVertex(radius * std::cos(a0), radius * std::sin(a0));
            addVertex(radius * std::cos(a1), radius * std::sin(a1));
        }
        return new Model(data.data(), static_cast<int>(data.size()), 9, 6);
    }

    // sphere.h stores position + normal (6 floats); lighting needs position + color + normal (9 floats)
    Model* createLitSphere(const glm::vec3& color)
    {
        const int count = static_cast<int>(sizeof(sphere) / sizeof(float));
        std::vector<float> data;
        data.reserve(count / 6 * 9);
        for (int i = 0; i + 5 < count; i += 6)
        {
            data.insert(data.end(), { sphere[i], sphere[i + 1], sphere[i + 2],
                color.r, color.g, color.b,
                sphere[i + 3], sphere[i + 4], sphere[i + 5] });
        }
        return new Model(data.data(), static_cast<int>(data.size()), 9, 3, 6);
    }

    Model* createSignatureModel()
    {
        return new Model(vyl0034_logo, static_cast<int>(sizeof(vyl0034_logo) / sizeof(float)));
    }
}

namespace
{
    constexpr float MOUSE_SENSITIVITY = 0.0025f;

    void cursorPosCallback(GLFWwindow* window, double x, double y)
    {
        static_cast<Application*>(glfwGetWindowUserPointer(window))->onCursorMoved(x, y);
    }

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int)
    {
        static_cast<Application*>(glfwGetWindowUserPointer(window))->onMouseButton(button, action);
    }

    void framebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        static_cast<Application*>(glfwGetWindowUserPointer(window))->onFramebufferResized(width, height);
    }
}

Application::Application(bool b)
    : broken(b), window(nullptr), basicProgram(nullptr), redProgram(nullptr), clockProgram(nullptr),
    constantProgram(nullptr), lambertProgram(nullptr), phongProgram(nullptr), blinnProgram(nullptr),
    phongWrongNormalsProgram(nullptr), phongLeakProgram(nullptr), phongClipProgram(nullptr),
    nightMatteProgram(nullptr), nightTreeProgram(nullptr), activeScene(0),
    triangleTransform(nullptr), translateFirst(true),
    resizeCorrect(true), framebufferWidth(0), framebufferHeight(0),
    lookActive(false), moveSpeed(1.5f), firstMouse(true), lastCursorX(0.0), lastCursorY(0.0), lastFrameTime(0.0) {}

Application::~Application()
{
    for (Scene* scene : scenes) delete scene;
    for (Model* model : models) delete model;
    delete basicProgram;
    delete redProgram;
    delete clockProgram;
    delete constantProgram;
    delete lambertProgram;
    delete phongProgram;
    delete blinnProgram;
    delete phongWrongNormalsProgram;
    delete phongLeakProgram;
    delete phongClipProgram;
    delete nightMatteProgram;
    delete nightTreeProgram;
}

void Application::initialization()
{
    if (!glfwInit()) std::exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1000, 700, "ZPG - Transformace a sceny", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::printf("GLAD initialization failed\n");
        std::exit(EXIT_FAILURE);
    }

    std::printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    std::printf("Renderer: %s\n", glGetString(GL_RENDERER));
    std::printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glEnable(GL_DEPTH_TEST);


    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    onFramebufferResized(width, height);
}

void Application::onMouseButton(int button, int action)
{
    if (button != GLFW_MOUSE_BUTTON_RIGHT) return;
    if (action == GLFW_PRESS)
    {
        lookActive = true;
        firstMouse = true;
    }
    else if (action == GLFW_RELEASE)
    {
        lookActive = false;
    }
}

void Application::onCursorMoved(double x, double y)
{
    if (!lookActive && !scenes[activeScene]->isFreeLook()) return;

    if (firstMouse)
    {
        lastCursorX = x;
        lastCursorY = y;
        firstMouse = false;
        return;
    }

    const float dx = static_cast<float>(x - lastCursorX);
    const float dy = static_cast<float>(y - lastCursorY);
    lastCursorX = x;
    lastCursorY = y;

    camera.rotate(dx * MOUSE_SENSITIVITY, dy * MOUSE_SENSITIVITY);
}

void Application::onFramebufferResized(int width, int height)
{
    if (width <= 0 || height <= 0) return;   // minimized window
    framebufferWidth = width;
    framebufferHeight = height;

    glViewport(0, 0, width, height);
    if (resizeCorrect)
    {
        // CORRECT: both the viewport and the aspect ratio of the projection match the new window size
        camera.setAspect(static_cast<float>(width) / static_cast<float>(height));
    }
    // WRONG (resizeCorrect == false): the viewport changes, but the projection keeps the old aspect ratio,
    // so the scene (e.g. the ball) gets distorted when the window is resized.
}

void Application::createShaders()
{
    basicProgram = new ShaderProgram("shaders/basic.vert", "shaders/basic.frag");

    const char* redFrag = broken ? "shaders/broken.frag" : "shaders/red.frag";
    redProgram = new ShaderProgram("shaders/basic.vert", redFrag);


    clockProgram = new ShaderProgram("shaders/clock.vert", "shaders/basic.frag");

    camera.attach(basicProgram);
    camera.attach(redProgram);
    camera.attach(clockProgram);

    // Four lighting models - shared vertex shader, different fragment shaders
    constantProgram = new ShaderProgram("shaders/lit.vert", "shaders/constant.frag");
    lambertProgram = new ShaderProgram("shaders/lit.vert", "shaders/lambert.frag");
    phongProgram = new ShaderProgram("shaders/lit.vert", "shaders/phong.frag");
    blinnProgram = new ShaderProgram("shaders/lit.vert", "shaders/blinn.frag");

    camera.attach(constantProgram);
    camera.attach(lambertProgram);
    camera.attach(phongProgram);
    camera.attach(blinnProgram);

    // Wrong normal conversion (mat3(M) * n) and Phong without / with clipping of the far-side highlight.
    // The shininess exponent is set to 1 so the highlight is wide and the far-side leak is clearly visible.
    phongWrongNormalsProgram = new ShaderProgram("shaders/lit_wrong.vert", "shaders/phong.frag");
    phongLeakProgram = new ShaderProgram("shaders/lit.vert", "shaders/phong_leak.frag");
    phongClipProgram = new ShaderProgram("shaders/lit.vert", "shaders/phong.frag");
    phongLeakProgram->use();
    phongLeakProgram->setUniform("shininess", 1.0f);
    phongClipProgram->use();
    phongClipProgram->setUniform("shininess", 1.0f);

    // Night forest: Phong with attenuation, two materials (matte and slightly shinier)
    nightMatteProgram = new ShaderProgram("shaders/lit.vert", "shaders/night.frag");
    nightTreeProgram = new ShaderProgram("shaders/lit.vert", "shaders/night.frag");
    nightMatteProgram->use();
    nightMatteProgram->setUniform("specularStrength", 0.02f);
    nightMatteProgram->setUniform("shininess", 4.0f);
    nightTreeProgram->use();
    nightTreeProgram->setUniform("specularStrength", 0.10f);
    nightTreeProgram->setUniform("shininess", 16.0f);

    camera.attach(phongWrongNormalsProgram);
    camera.attach(phongLeakProgram);
    camera.attach(phongClipProgram);
    camera.attach(nightMatteProgram);
    camera.attach(nightTreeProgram);

    litPrograms = { constantProgram, lambertProgram, phongProgram, blinnProgram,
        phongWrongNormalsProgram, phongLeakProgram, phongClipProgram, nightMatteProgram, nightTreeProgram };
}

namespace
{
    enum ModelId
    {
        TRIANGLE, SPHERE, SIGNATURE, SUN_BALL, EARTH, MOON, CLOCK,
        GROUND, POST,
        TREE_CONIFER_DARK, TREE_CONIFER_LIGHT, TREE_ROUND_MID, TREE_ROUND_LIGHT,
        BUSH_DARK, BUSH_LIGHT,
        ROCKET, LIT_SPHERE, MERCURY, VENUS, MARS, JUPITER, LIGHT_MARKER,
        NIGHT_GROUND, NIGHT_CONIFER_DARK, NIGHT_CONIFER_LIGHT, NIGHT_ROUND_MID, NIGHT_ROUND_LIGHT,
        NIGHT_BUSH_DARK, NIGHT_BUSH_LIGHT, FIREFLY
    };
}

void Application::createModels()
{
    models.push_back(createTriangleModel()); // TRIANGLE
    models.push_back(new Model(sphere, sizeof(sphere) / sizeof(float))); // SPHERE
    models.push_back(createSignatureModel()); // SIGNATURE
    models.push_back(new Model(sun, sizeof(sun) / sizeof(float), 9, 3, 6)); // SUN (position, color, normal)
    models.push_back(new Model(earth, sizeof(earth) / sizeof(float), 9, 3, 6)); // EARTH
    models.push_back(new Model(moon, sizeof(moon) / sizeof(float), 9, 3, 6)); // MOON
    models.push_back(createClockModel()); // CLOCK (position, normal, color + dial)

    models.push_back(makeModel(proc::ground(8, 7))); // GROUND
    models.push_back(makeModel(proc::post())); // POST
    models.push_back(makeModel(proc::conifer(glm::vec3(0.08f, 0.36f, 0.13f)))); // dark conifer
    models.push_back(makeModel(proc::conifer(glm::vec3(0.16f, 0.50f, 0.17f)))); // lighter conifer
    models.push_back(makeModel(proc::broadleaf(glm::vec3(0.20f, 0.52f, 0.14f)))); // broadleaf tree
    models.push_back(makeModel(proc::broadleaf(glm::vec3(0.34f, 0.62f, 0.20f)))); // light broadleaf tree
    models.push_back(makeModel(proc::bush(glm::vec3(0.12f, 0.42f, 0.14f)))); // dark bush
    models.push_back(makeModel(proc::bush(glm::vec3(0.26f, 0.55f, 0.18f)))); // light bush

    {
        const proc::Data rocket = proc::rocket();
        models.push_back(new Model(rocket.data(), static_cast<int>(rocket.size()), 9, 3, 6)); // ROCKET
    }
    models.push_back(createLitSphere(glm::vec3(0.25f, 0.55f, 0.95f)));  // LIT_SPHERE (test sphere)
    models.push_back(createLitSphere(glm::vec3(0.62f, 0.58f, 0.55f)));  // MERCURY
    models.push_back(createLitSphere(glm::vec3(0.90f, 0.75f, 0.45f)));  // VENUS
    models.push_back(createLitSphere(glm::vec3(0.80f, 0.30f, 0.15f)));  // MARS
    models.push_back(createLitSphere(glm::vec3(0.80f, 0.62f, 0.42f)));  // JUPITER
    models.push_back(createLitSphere(glm::vec3(1.00f, 0.95f, 0.60f)));  // LIGHT_MARKER

    // Lit versions (with normals, pure material colors) of the forest models for the night scene.
    // The order must match the ModelId enum.
    models.push_back(makeLitModel([] { return proc::ground(8, 7); }));                                // NIGHT_GROUND
    models.push_back(makeLitModel([] { return proc::conifer(glm::vec3(0.08f, 0.36f, 0.13f)); }));     // NIGHT_CONIFER_DARK
    models.push_back(makeLitModel([] { return proc::conifer(glm::vec3(0.16f, 0.50f, 0.17f)); }));     // NIGHT_CONIFER_LIGHT
    models.push_back(makeLitModel([] { return proc::broadleaf(glm::vec3(0.20f, 0.52f, 0.14f)); }));   // NIGHT_ROUND_MID
    models.push_back(makeLitModel([] { return proc::broadleaf(glm::vec3(0.34f, 0.62f, 0.20f)); }));   // NIGHT_ROUND_LIGHT
    models.push_back(makeLitModel([] { return proc::bush(glm::vec3(0.12f, 0.42f, 0.14f)); }));        // NIGHT_BUSH_DARK
    models.push_back(makeLitModel([] { return proc::bush(glm::vec3(0.26f, 0.55f, 0.18f)); }));        // NIGHT_BUSH_LIGHT
    models.push_back(createLitSphere(glm::vec3(1.00f, 1.00f, 0.55f)));                                // FIREFLY (glowing ball)
}

void Application::createScenes()
{
    const glm::vec3 axisY(0, 1, 0);
    const glm::vec3 axisZ(0, 0, 1);

    Scene* triangle = new Scene("Trojuhelnik");
    triangleTransform = makeComposite({
        std::make_shared<Translate>(glm::vec3(0.5f, 0.0f, 0.0f)),
        std::make_shared<DynamicRotate>(axisZ, 1.2f),
        std::make_shared<Scale>(0.45f)
        });
    translateFirst = true;
    triangle->add(models[TRIANGLE], basicProgram, triangleTransform);
    triangle->add(models[SPHERE], redProgram, place(0, 0, -0.5f, 0.03f));
    triangle->add(models[SIGNATURE], basicProgram, place(0.68f, -0.78f, -0.9f, 0.18f));
    scenes.push_back(triangle);


    Scene* ball = new Scene("Kulicka");
    ball->add(models[SPHERE], redProgram, place(0, 0, 0, 0.75f));
    ball->add(models[SIGNATURE], basicProgram, place(0.68f, -0.78f, -0.9f, 0.18f));
    scenes.push_back(ball);


    Scene* forest = new Scene("Les");
    forest->setCameraStart(glm::vec3(0.0f, 1.7f, 14.0f), -PI / 2.0f, PI / 2.0f, 4.0f);
    forest->setFreeLook(true);
    std::mt19937 rng(34);
    auto random = [&rng](float lo, float hi) {
        return std::uniform_real_distribution<float>(lo, hi)(rng);
        };

    forest->add(models[GROUND], basicProgram, makeComposite({ std::make_shared<Scale>(45.0f) }));

    struct ClockSpot { float x, z, yaw; };
    const ClockSpot clockSpots[] = {
        { -4.0f,   6.0f, PI + 0.35f },
        {  7.0f,  -5.0f, PI - 0.60f },
        { -13.0f, -15.0f, PI + 0.90f },
    };
    const float clockHeight = 2.4f;
    for (const ClockSpot& c : clockSpots)
    {
        forest->add(models[CLOCK], clockProgram, placeY(c.x, clockHeight, c.z, 1.0f, c.yaw));
        forest->add(models[POST], basicProgram, makeComposite({
            std::make_shared<Translate>(glm::vec3(c.x, 0.0f, c.z)),
            std::make_shared<Rotate>(c.yaw, glm::vec3(0, 1, 0)),
            std::make_shared<Translate>(glm::vec3(0.0f, 0.0f, 0.15f)),
            std::make_shared<Scale>(clockHeight)
            }));
    }

    const glm::vec2 keepOut[] = {
        { clockSpots[0].x, clockSpots[0].z }, { clockSpots[1].x, clockSpots[1].z },
        { clockSpots[2].x, clockSpots[2].z }, { 2.5f, 8.0f }, { 0.0f, 14.0f }
    };
    auto freeSpot = [&](float x, float z) {
        for (const glm::vec2& k : keepOut)
            if (glm::distance(glm::vec2(x, z), k) < 3.0f) return false;
        return true;
        };

    const int TREE_COUNT = 80;
    const int BUSH_COUNT = 40;
    const ModelId treeModels[] = { TREE_CONIFER_DARK, TREE_CONIFER_LIGHT, TREE_ROUND_MID, TREE_ROUND_LIGHT };
    const ModelId bushModels[] = { BUSH_DARK, BUSH_LIGHT };

    for (int i = 0; i < TREE_COUNT; ++i)
    {
        float x, z;
        do { x = random(-43.0f, 43.0f); z = random(-43.0f, 43.0f); } while (!freeSpot(x, z));
        forest->add(models[treeModels[rng() % 4]], basicProgram,
            placeY(x, 0.0f, z, random(0.8f, 1.8f), random(0.0f, 2.0f * PI)));
    }
    for (int i = 0; i < BUSH_COUNT; ++i)
    {
        float x, z;
        do { x = random(-43.0f, 43.0f); z = random(-43.0f, 43.0f); } while (!freeSpot(x, z));
        forest->add(models[bushModels[rng() % 2]], basicProgram,
            placeY(x, 0.0f, z, random(0.7f, 1.7f), random(0.0f, 2.0f * PI)));
    }

    forest->add(models[SUN_BALL], basicProgram, makeComposite({
        std::make_shared<Translate>(glm::vec3(30.0f, 22.0f, -55.0f)),
        std::make_shared<DynamicRotate>(axisY, 0.4f),
        std::make_shared<DynamicScale>([](float t) {
            return glm::vec3(4.0f * (1.0f + 0.1f * std::sin(2.0f * t)));
        })
        }));
    forest->add(models[SIGNATURE], basicProgram, placeY(2.5f, 0.5f, 8.0f, 1.2f, 0.0f));
    scenes.push_back(forest);

    Scene* signature = new Scene("Vlastni login");
    signature->add(models[SIGNATURE], basicProgram, place(0, 0, 0, 0.75f));
    signature->add(models[SPHERE], redProgram, place(0, -0.68f, 0, 0.14f));
    scenes.push_back(signature);


    // ---------------------------------------------------------------
    // Solar system with lighting: the Sun is the light source at the origin [0,0,0]
    // ---------------------------------------------------------------
    Scene* solar = new Scene("Slunecni soustava");
    solar->setLight(glm::vec3(0.0f, 0.0f, 0.0f));
    solar->setCameraStart(glm::vec3(0.0f, 0.0f, 3.4f), -PI / 2.0f, PI / 2.0f, 1.5f);

    // Sun - constant shader (uniformly visible, independent of light direction)
    solar->add(models[SUN_BALL], constantProgram, makeComposite({
        std::make_shared<DynamicRotate>(axisY, 0.4f),
        std::make_shared<Scale>(0.25f)
        }));

    // Planets on circular orbits (rotation around Z * translation by the orbit radius)
    auto planet = [&](ModelId id, ShaderProgram* program, float orbitRadius, float orbitSpeed,
        float phase, float size)
        {
            solar->add(models[id], program, makeComposite({
                std::make_shared<DynamicRotate>(axisZ, orbitSpeed, phase),
                std::make_shared<Translate>(glm::vec3(orbitRadius, 0.0f, 0.0f)),
                std::make_shared<Scale>(size)
                }));
        };
    planet(MERCURY, phongProgram, 0.45f, 1.6f, 0.5f, 0.04f);
    planet(VENUS, blinnProgram, 0.62f, 1.0f, 2.4f, 0.07f);
    planet(MARS, phongProgram, 1.00f, 0.35f, 4.0f, 0.06f);
    planet(JUPITER, blinnProgram, 1.30f, 0.20f, 1.0f, 0.16f);

    // Earth (Phong) + Moon (Blinn-Phong)
    CompositePtr earthOrbit = makeComposite({
        std::make_shared<DynamicRotate>(axisZ, 0.5f),
        std::make_shared<Translate>(glm::vec3(0.82f, 0.0f, 0.0f))
        });
    solar->add(models[EARTH], phongProgram, makeComposite({
        earthOrbit,
        std::make_shared<DynamicRotate>(axisY, 2.5f),
        std::make_shared<Scale>(0.09f)
        }));
    solar->add(models[MOON], blinnProgram, makeComposite({
        earthOrbit,
        std::make_shared<DynamicRotate>(axisZ, 3.0f),
        std::make_shared<Translate>(glm::vec3(0.17f, 0.0f, 0.0f)),
        std::make_shared<DynamicRotate>(axisY, 1.0f),
        std::make_shared<Scale>(0.035f)
        }));

    // Rockets (procedurally generated model, nose points to +Y; rotation around Z turns it along the orbit tangent).
    // 1) rocket orbits the Earth in a tilted plane
    solar->add(models[ROCKET], phongProgram, makeComposite({
        earthOrbit,
        std::make_shared<Rotate>(0.8f, glm::vec3(1, 0, 0)),
        std::make_shared<DynamicRotate>(axisZ, 3.0f),
        std::make_shared<Translate>(glm::vec3(0.26f, 0.0f, 0.0f)),
        std::make_shared<Scale>(0.07f)
        }));
    // 2) rocket orbits the Sun between Mars and Jupiter in a different plane
    solar->add(models[ROCKET], phongProgram, makeComposite({
        std::make_shared<Rotate>(0.5f, glm::vec3(1, 0, 0)),
        std::make_shared<DynamicRotate>(axisZ, 0.9f, 1.5f),
        std::make_shared<Translate>(glm::vec3(1.15f, 0.0f, 0.0f)),
        std::make_shared<Scale>(0.09f)
        }));

    solar->add(models[SIGNATURE], basicProgram, place(1.45f, -1.2f, 0.9f, 0.2f));
    scenes.push_back(solar);

    // ---------------------------------------------------------------
    // Test scene: four spheres, point light in the middle [0,0,0]
    // around the light: constant, Lambert, Phong, Blinn-Phong
    // ---------------------------------------------------------------
    Scene* spheres = new Scene("Ctyri koule: konstantni / Lambert / Phong / Blinn");
    spheres->setLight(glm::vec3(0.0f, 0.0f, 0.0f));
    {
        const glm::vec3 eye(0.0f, 8.0f, 3.0f);
        const glm::vec3 dir = glm::normalize(-eye);          // look at the origin
        spheres->setCameraStart(eye, std::atan2(dir.z, dir.x), std::acos(dir.y), 3.0f);
    }
    spheres->add(models[LIT_SPHERE], constantProgram, place(-3.0f, 0.0f, 0.0f, 1.0f));
    spheres->add(models[LIT_SPHERE], lambertProgram, place(0.0f, 0.0f, -3.0f, 1.0f));
    spheres->add(models[LIT_SPHERE], phongProgram, place(3.0f, 0.0f, 0.0f, 1.0f));
    spheres->add(models[LIT_SPHERE], blinnProgram, place(0.0f, 0.0f, 3.0f, 1.0f));
    spheres->add(models[LIGHT_MARKER], constantProgram, place(0.0f, 0.0f, 0.0f, 0.15f)); // light position marker
    scenes.push_back(spheres);

    Scene* clockScene = new Scene("Hodiny");
    clockScene->add(models[CLOCK], clockProgram, placeY(0, 0, 0, 0.9f, PI));
    clockScene->add(models[SIGNATURE], basicProgram, place(0.68f, -0.88f, -0.95f, 0.14f));
    scenes.push_back(clockScene);

    // ---------------------------------------------------------------
    // 1) Normal conversion: left WRONG (mat3(M) * n), right CORRECT (transpose(inverse(mat3(M))) * n).
    //    Non-uniformly scaled ellipsoids: the wrong normal is not perpendicular to the surface,
    //    so both the diffuse shading and the highlight are wrong.
    // ---------------------------------------------------------------
    Scene* normals = new Scene("Normaly: vlevo CHYBNE, vpravo SPRAVNE");
    normals->setLight(glm::vec3(0.0f, 2.5f, 3.0f));
    {
        const glm::vec3 eye(0.0f, 0.0f, 7.0f);
        normals->setCameraStart(eye, -PI / 2.0f, PI / 2.0f, 3.0f);
    }
    auto ellipsoid = [&](float x) {
        return makeComposite({
            std::make_shared<Translate>(glm::vec3(x, 0.0f, 0.0f)),
            std::make_shared<Rotate>(0.6f, axisZ),
            std::make_shared<Scale>(glm::vec3(1.6f, 0.5f, 0.8f))   // non-uniform scale
            });
        };
    normals->add(models[LIT_SPHERE], phongWrongNormalsProgram, ellipsoid(-2.6f));   // wrong normals
    normals->add(models[LIT_SPHERE], phongProgram, ellipsoid(2.6f));                // correct normals (lit.vert)
    normals->add(models[LIGHT_MARKER], constantProgram, place(0.0f, 2.5f, 3.0f, 0.12f));
    scenes.push_back(normals);

    // ---------------------------------------------------------------
    // 2) Far side of the ball, shininess = 1, light placed BEHIND the balls:
    //    left WRONG (specular term also on the far side), right CORRECT (clipped by diff > 0)
    // ---------------------------------------------------------------
    Scene* backSide = new Scene("Odvracena strana: vlevo CHYBNE (presvit), vpravo SPRAVNE (orezano)");
    backSide->setLight(glm::vec3(0.0f, 0.0f, -3.0f));
    {
        const glm::vec3 eye(0.0f, 0.0f, 7.0f);
        backSide->setCameraStart(eye, -PI / 2.0f, PI / 2.0f, 3.0f);
    }
    backSide->add(models[LIT_SPHERE], phongLeakProgram, place(-1.6f, 0.0f, 0.0f, 1.0f));   // no clipping
    backSide->add(models[LIT_SPHERE], phongClipProgram, place(1.6f, 0.0f, 0.0f, 1.0f));    // clipped
    backSide->add(models[LIGHT_MARKER], constantProgram, place(0.0f, 0.0f, -3.0f, 0.12f));
    scenes.push_back(backSide);

    // ---------------------------------------------------------------
    // 3) Night forest with ONE point light (firefly / lantern).
    //    - the light is attenuated: 1 / (kc + kl * d + kq * d^2) -> only its close surroundings are lit,
    //      the intensity keeps falling with the distance
    //    - the rest of the forest stays almost dark (very weak ambient term Ia + distance fog)
    //    - materials: matte ground and bushes (weak specular), trees slightly shinier
    // ---------------------------------------------------------------
    Scene* night = new Scene("Nocni les - svetluska");
    night->setCameraStart(glm::vec3(0.0f, 1.7f, 14.0f), -PI / 2.0f, PI / 2.0f, 4.0f);
    night->setFreeLook(true);
    night->setClearColor(glm::vec3(0.004f, 0.006f, 0.012f));   // almost black night, same as the fog color

    // the light hovers slowly above the ground in a small clearing in front of the camera
    const glm::vec3 clearing(0.0f, 0.0f, 7.0f);
    const auto fireflyPath = [clearing](float t) {
        return clearing + glm::vec3(1.2f * std::sin(0.35f * t), 1.2f + 0.25f * std::sin(1.1f * t), 1.0f * std::cos(0.5f * t));
        };
    night->setLight(fireflyPath(0.0f), glm::vec3(1.0f, 0.85f, 0.45f) * 3.0f);   // warm yellow light (Id, Is)
    night->setLightPath(fireflyPath);
    // kc = 1, kl = 0.7, kq = 1.8: strong near the source, ~10 % at 2 m, ~3 % at 3 m, ~1 % at 7 m
    night->setLightAttenuation(glm::vec3(1.0f, 0.7f, 1.8f));

    night->add(models[NIGHT_GROUND], nightMatteProgram, makeComposite({ std::make_shared<Scale>(45.0f) }));

    std::mt19937 nightRng(7);   // fixed seed -> the same forest every run
    auto nightRandom = [&nightRng](float lo, float hi) {
        return std::uniform_real_distribution<float>(lo, hi)(nightRng);
        };
    const ModelId nightTrees[] = { NIGHT_CONIFER_DARK, NIGHT_CONIFER_LIGHT, NIGHT_ROUND_MID, NIGHT_ROUND_LIGHT };
    const ModelId nightBushes[] = { NIGHT_BUSH_DARK, NIGHT_BUSH_LIGHT };

    // trees and bushes standing around the light - their trunks and the ground between them get lit by it
    for (int i = 0; i < 8; ++i)
    {
        const float angle = 2.0f * PI * i / 8.0f + nightRandom(-0.2f, 0.2f);
        const float radius = nightRandom(3.0f, 5.5f);
        night->add(models[nightTrees[nightRng() % 4]], nightTreeProgram,
            placeY(clearing.x + radius * std::cos(angle), 0.0f, clearing.z + radius * std::sin(angle),
                nightRandom(1.2f, 2.0f), nightRandom(0.0f, 2.0f * PI)));
    }
    for (int i = 0; i < 6; ++i)
    {
        const float angle = 2.0f * PI * i / 6.0f + nightRandom(-0.3f, 0.3f);
        const float radius = nightRandom(2.2f, 4.0f);
        night->add(models[nightBushes[nightRng() % 2]], nightMatteProgram,
            placeY(clearing.x + radius * std::cos(angle), 0.0f, clearing.z + radius * std::sin(angle),
                nightRandom(0.7f, 1.3f), nightRandom(0.0f, 2.0f * PI)));
    }

    // the rest of the forest (dense, but almost invisible in the dark)
    auto nightFreeSpot = [&](float x, float z) {
        return glm::distance(glm::vec2(x, z), glm::vec2(clearing.x, clearing.z)) > 3.0f     // keep the clearing free
            && glm::distance(glm::vec2(x, z), glm::vec2(0.0f, 14.0f)) > 2.5f;               // and the camera start
        };
    for (int i = 0; i < 140; ++i)
    {
        float x, z;
        do { x = nightRandom(-22.0f, 22.0f); z = nightRandom(-22.0f, 22.0f); } while (!nightFreeSpot(x, z));
        night->add(models[nightTrees[nightRng() % 4]], nightTreeProgram,
            placeY(x, 0.0f, z, nightRandom(1.0f, 2.2f), nightRandom(0.0f, 2.0f * PI)));
    }
    for (int i = 0; i < 80; ++i)
    {
        float x, z;
        do { x = nightRandom(-22.0f, 22.0f); z = nightRandom(-22.0f, 22.0f); } while (!nightFreeSpot(x, z));
        night->add(models[nightBushes[nightRng() % 2]], nightMatteProgram,
            placeY(x, 0.0f, z, nightRandom(0.7f, 1.7f), nightRandom(0.0f, 2.0f * PI)));
    }

    // the firefly itself - a small glowing ball (constant shader) following the same path as the light
    night->add(models[FIREFLY], constantProgram, makeComposite({
        std::make_shared<DynamicTranslate>(fireflyPath),
        std::make_shared<Scale>(0.06f)
        }));
    scenes.push_back(night);
}

void Application::updateTitle()
{
    std::string title = "ZPG - " + scenes[activeScene]->getName();
    if (activeScene == 0)
        title += translateFirst ? " | poradi: posun, rotace (T*R)"
        : " | poradi: rotace, posun (R*T)";
    // show the current resize mode (toggled with the R key)
    title += resizeCorrect ? " | okno: spravne (R)" : " | okno: CHYBNE (R)";
    glfwSetWindowTitle(window, title.c_str());
}

void Application::updateClock()
{
    using namespace std::chrono;
    const auto now = system_clock::now();
    const std::time_t t = system_clock::to_time_t(now);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    const double fraction = static_cast<double>(duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000) / 1000.0;
    const float seconds = static_cast<float>(local.tm_hour * 3600 + local.tm_min * 60 + local.tm_sec + fraction);

    clockProgram->use();
    clockProgram->setUniform("clockSeconds", seconds);
}

// Sends the scene light (position, color, attenuation) to every program that uses lighting.
// Called every frame because the light can move (firefly).
void Application::applyLight(const Scene& scene, float time)
{
    for (ShaderProgram* program : litPrograms)
    {
        program->use();
        // not every lit shader has every uniform (e.g. attenuation exists only in night.frag)
        if (program->hasUniform("lightPosition")) program->setUniform("lightPosition", scene.getLightPosition(time));
        if (program->hasUniform("lightColor")) program->setUniform("lightColor", scene.getLightColor());
        if (program->hasUniform("lightAttenuation")) program->setUniform("lightAttenuation", scene.getLightAttenuation());
    }
}

void Application::setScene(int index)
{
    if (index >= 0 && index < static_cast<int>(scenes.size()))
    {
        activeScene = index;
        const Scene* scene = scenes[activeScene];
        camera.setPose(scene->getCameraEye(), scene->getCameraFi(), scene->getCameraAlpha());
        moveSpeed = scene->getMoveSpeed();
        applyLight(*scene, static_cast<float>(glfwGetTime()));
        glfwSetInputMode(window, GLFW_CURSOR, scene->isFreeLook() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
        lookActive = false;
        std::cout << "Aktivni scena: " << activeScene + 1 << " - "
            << scenes[activeScene]->getName() << std::endl;
        updateTitle();
    }
}

void Application::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    const int sceneCount = static_cast<int>(scenes.size());
    static bool previous[10] = {};
    for (int i = 0; i < sceneCount && i < 10; ++i)
    {
        // scenes 1-9 are on keys 1-9, scene 10 is on key 0
        const int key = (i < 9) ? GLFW_KEY_1 + i : GLFW_KEY_0;
        bool pressed = glfwGetKey(window, key) == GLFW_PRESS;
        if (pressed && !previous[i]) setScene(i);
        previous[i] = pressed;
    }


    const double now = glfwGetTime();
    const float step = moveSpeed * static_cast<float>(now - lastFrameTime);
    lastFrameTime = now;
    float forward = 0.0f, right = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) forward += step;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) forward -= step;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) right += step;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) right -= step;
    if (forward != 0.0f || right != 0.0f) camera.move(forward, right);

    // R: toggle between correct and wrong viewport / projection update on window resize
    static bool resizeKeyPrevious = false;
    const bool resizeKey = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (resizeKey && !resizeKeyPrevious)
    {
        resizeCorrect = !resizeCorrect;
        std::cout << (resizeCorrect ? "Zmena velikosti okna: SPRAVNE (viewport + perspektiva)"
            : "Zmena velikosti okna: CHYBNE (jen viewport, perspektiva se neaktualizuje)") << std::endl;
        // when switching back to the correct mode, apply the current window size immediately
        if (resizeCorrect) onFramebufferResized(framebufferWidth, framebufferHeight);
        updateTitle();
    }
    resizeKeyPrevious = resizeKey;

    static bool spacePrevious = false;
    bool space = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (space && !spacePrevious && triangleTransform)
    {
        triangleTransform->swap(0, 1);
        translateFirst = !translateFirst;
        std::cout << (translateFirst ? "Poradi: posun, rotace (T*R)"
            : "Poradi: rotace, posun (R*T)") << std::endl;
        updateTitle();
    }
    spacePrevious = space;
}

void Application::run()
{
    if (!scenes.empty()) setScene(activeScene);
    lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        processInput();

        // every scene can have its own background color (the night forest is almost black)
        const glm::vec3 clearColor = scenes.empty() ? glm::vec3(0.035f, 0.045f, 0.08f) : scenes[activeScene]->getClearColor();
        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateClock();
        if (!scenes.empty())
        {
            const float time = static_cast<float>(glfwGetTime());
            applyLight(*scenes[activeScene], time);    // the light can move (firefly), so update it every frame
            scenes[activeScene]->draw(time);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
}