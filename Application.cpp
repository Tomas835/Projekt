#include "Application.h"
#include "models/clock.h"
#include "models/earth.h"
#include "models/moon.h"
#include "models/sphere.h"
#include "models/sun.h"
#include "models/vyl0034_logo.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
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

    Model* createTreeModel()
    {
        std::vector<float> d;
        const float w = 0.12f;
        addTriangle(d, -w, -0.75f, 0.0f, w, -0.75f, 0.0f, w, 0.0f, 0.0f,
            0.48f, 0.25f, 0.08f);
        addTriangle(d, -w, -0.75f, 0.0f, w, 0.0f, 0.0f, -w, 0.0f, 0.0f,
            0.48f, 0.25f, 0.08f);
        addTriangle(d, 0.0f, 0.72f, 0.0f, -0.52f, -0.08f, 0.0f, 0.52f, -0.08f, 0.0f,
            0.08f, 0.48f, 0.12f);
        addTriangle(d, 0.0f, 0.42f, 0.0f, -0.42f, -0.30f, 0.0f, 0.42f, -0.30f, 0.0f,
            0.05f, 0.38f, 0.08f);
        return makeModel(d);
    }

    Model* createBushModel()
    {
        std::vector<float> d;
        addTriangle(d, -0.55f, -0.35f, 0, 0.0f, 0.45f, 0, 0.55f, -0.35f, 0,
            0.10f, 0.55f, 0.12f);
        addTriangle(d, -0.45f, -0.30f, 0.15f, 0.0f, 0.38f, 0.15f, 0.45f, -0.30f, 0.15f,
            0.16f, 0.68f, 0.18f);
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

    Model* createSignatureModel()
    {
        return new Model(vyl0034_logo, static_cast<int>(sizeof(vyl0034_logo) / sizeof(float)));
    }
}

namespace
{
    constexpr float MOUSE_SENSITIVITY = 0.0025f; // radianu na pixel
    constexpr float MOVE_SPEED = 1.5f;           // jednotek za sekundu

    void cursorPosCallback(GLFWwindow* window, double x, double y)
    {
        static_cast<Application*>(glfwGetWindowUserPointer(window))->onCursorMoved(x, y);
    }

    void framebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        static_cast<Application*>(glfwGetWindowUserPointer(window))->onFramebufferResized(width, height);
    }
}

Application::Application(bool b)
    : broken(b), window(nullptr), basicProgram(nullptr), redProgram(nullptr), activeScene(0),
    triangleTransform(nullptr), translateFirst(true),
    firstMouse(true), lastCursorX(0.0), lastCursorY(0.0), lastFrameTime(0.0) {}

Application::~Application()
{
    for (Scene* scene : scenes) delete scene;
    for (Model* model : models) delete model;
    delete basicProgram;
    delete redProgram;
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

    // Mys (rozhlizeni kamery) a zmena velikosti okna (pomer stran projekce).
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    onFramebufferResized(width, height);
}

void Application::onCursorMoved(double x, double y)
{
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
    if (width <= 0 || height <= 0) return; // minimalizovane okno
    glViewport(0, 0, width, height);
    camera.setAspect(static_cast<float>(width) / static_cast<float>(height));
}

void Application::createShaders()
{
    basicProgram = new ShaderProgram("shaders/basic.vert", "shaders/basic.frag");

    const char* redFrag = broken ? "shaders/broken.frag" : "shaders/red.frag";
    redProgram = new ShaderProgram("shaders/basic.vert", redFrag);

    // Kamera posila view a projection matici obema shader programum.
    camera.attach(basicProgram);
    camera.attach(redProgram);
}

namespace
{
    enum ModelId
    {
        TRIANGLE, SPHERE, TREE, BUSH, SIGNATURE,
        SUN_BALL, EARTH, MOON, CLOCK
    };
}

void Application::createModels()
{
    models.push_back(createTriangleModel()); // TRIANGLE
    models.push_back(new Model(sphere, sizeof(sphere) / sizeof(float))); // SPHERE
    models.push_back(createTreeModel()); // TREE
    models.push_back(createBushModel()); // BUSH
    models.push_back(createSignatureModel()); // SIGNATURE
    models.push_back(new Model(sun, sizeof(sun) / sizeof(float), 9)); // SUN
    models.push_back(new Model(earth, sizeof(earth) / sizeof(float), 9)); // EARTH
    models.push_back(new Model(moon, sizeof(moon) / sizeof(float), 9)); // MOON
    models.push_back(new Model(clockModel, sizeof(clockModel) / sizeof(float), 9, 6)); // CLOCK (pozice, normala, barva)
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
    std::mt19937 rng(34);
    auto random = [&rng](float lo, float hi) {
        return std::uniform_real_distribution<float>(lo, hi)(rng);
        };
    for (int i = 0; i < 24; ++i)
    {
        const float x = random(-0.85f, 0.85f);
        const float y = random(-0.45f, 0.55f);
        forest->add(models[TREE], basicProgram,
            place(x, y, 0.5f * y, random(0.18f, 0.30f), random(-0.07f, 0.07f)));
    }
    for (int i = 0; i < 24; ++i)
    {
        const float x = random(-0.85f, 0.85f);
        const float y = random(-0.95f, 0.10f);
        forest->add(models[BUSH], basicProgram,
            place(x, y, 0.5f * y, random(0.08f, 0.15f), random(-0.1f, 0.1f)));
    }


    forest->add(models[SUN_BALL], basicProgram, makeComposite({
        std::make_shared<Translate>(glm::vec3(0.72f, 0.72f, 0.6f)),
        std::make_shared<DynamicRotate>(axisY, 0.4f),
        std::make_shared<DynamicScale>([](float t) {
            return glm::vec3(0.16f * (1.0f + 0.1f * std::sin(2.0f * t)));
        })
        }));
    forest->add(models[SIGNATURE], basicProgram, place(0.72f, -0.82f, -0.95f, 0.16f));
    scenes.push_back(forest);

    Scene* signature = new Scene("Vlastni login");
    signature->add(models[SIGNATURE], basicProgram, place(0, 0, 0, 0.75f));
    signature->add(models[SPHERE], redProgram, place(0, -0.68f, 0, 0.14f));
    scenes.push_back(signature);


    Scene* solar = new Scene("Slunecni soustava");
    solar->add(models[SUN_BALL], basicProgram, makeComposite({
        std::make_shared<DynamicRotate>(axisY, 0.4f),
        std::make_shared<Scale>(0.25f)
        }));

    CompositePtr earthOrbit = makeComposite({
        std::make_shared<DynamicRotate>(axisZ, 0.5f),
        std::make_shared<Translate>(glm::vec3(0.72f, 0.0f, 0.0f))
        });
    solar->add(models[EARTH], basicProgram, makeComposite({
        earthOrbit,
        std::make_shared<DynamicRotate>(axisY, 2.5f),
        std::make_shared<Scale>(0.09f)
        }));
    solar->add(models[MOON], basicProgram, makeComposite({
        earthOrbit,
        std::make_shared<DynamicRotate>(axisZ, 3.0f),
        std::make_shared<Translate>(glm::vec3(0.17f, 0.0f, 0.0f)),
        std::make_shared<DynamicRotate>(axisY, 1.0f),
        std::make_shared<Scale>(0.035f)
        }));
    solar->add(models[SIGNATURE], basicProgram, place(0.68f, -0.88f, -0.95f, 0.14f));
    scenes.push_back(solar);

    Scene* clockScene = new Scene("Hodiny");
    clockScene->add(models[CLOCK], basicProgram, place(0, 0, 0, 0.9f));
    clockScene->add(models[SIGNATURE], basicProgram, place(0.68f, -0.88f, -0.95f, 0.14f));
    scenes.push_back(clockScene);
}

void Application::updateTitle()
{
    std::string title = "ZPG - " + scenes[activeScene]->getName();
    if (activeScene == 0)
        title += translateFirst ? " | poradi: posun, rotace (T*R)"
        : " | poradi: rotace, posun (R*T)";
    glfwSetWindowTitle(window, title.c_str());
}

void Application::setScene(int index)
{
    if (index >= 0 && index < static_cast<int>(scenes.size()))
    {
        activeScene = index;
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
    static bool previous[9] = {};
    for (int i = 0; i < sceneCount && i < 9; ++i)
    {
        bool pressed = glfwGetKey(window, GLFW_KEY_1 + i) == GLFW_PRESS;
        if (pressed && !previous[i]) setScene(i);
        previous[i] = pressed;
    }

    // Pohyb kamery: W/S dopredu/dozadu, A/D do stran.
    const double now = glfwGetTime();
    const float step = MOVE_SPEED * static_cast<float>(now - lastFrameTime);
    lastFrameTime = now;
    float forward = 0.0f, right = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) forward += step;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) forward -= step;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) right += step;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) right -= step;
    if (forward != 0.0f || right != 0.0f) camera.move(forward, right);

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
    if (!scenes.empty()) updateTitle();
    lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        processInput();

        glClearColor(0.035f, 0.045f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!scenes.empty())
            scenes[activeScene]->draw(static_cast<float>(glfwGetTime()));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
}