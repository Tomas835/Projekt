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
    : broken(b), window(nullptr), basicProgram(nullptr), redProgram(nullptr), clockProgram(nullptr), activeScene(0),
    triangleTransform(nullptr), translateFirst(true),
    lookActive(false), moveSpeed(1.5f), firstMouse(true), lastCursorX(0.0), lastCursorY(0.0), lastFrameTime(0.0) {}

Application::~Application()
{
    for (Scene* scene : scenes) delete scene;
    for (Model* model : models) delete model;
    delete basicProgram;
    delete redProgram;
    delete clockProgram;
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
    if (!lookActive) return;

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
    if (width <= 0 || height <= 0) return;
    glViewport(0, 0, width, height);
    camera.setAspect(static_cast<float>(width) / static_cast<float>(height));
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
}

namespace
{
    enum ModelId
    {
        TRIANGLE, SPHERE, SIGNATURE, SUN_BALL, EARTH, MOON, CLOCK,
        GROUND, POST,
        TREE_CONIFER_DARK, TREE_CONIFER_LIGHT, TREE_ROUND_MID, TREE_ROUND_LIGHT,
        BUSH_DARK, BUSH_LIGHT
    };
}

void Application::createModels()
{
    models.push_back(createTriangleModel()); // TRIANGLE
    models.push_back(new Model(sphere, sizeof(sphere) / sizeof(float))); // SPHERE
    models.push_back(createSignatureModel()); // SIGNATURE
    models.push_back(new Model(sun, sizeof(sun) / sizeof(float), 9)); // SUN
    models.push_back(new Model(earth, sizeof(earth) / sizeof(float), 9)); // EARTH
    models.push_back(new Model(moon, sizeof(moon) / sizeof(float), 9)); // MOON
    models.push_back(createClockModel()); // CLOCK (pozice, normala, barva + cifernik)

    models.push_back(makeModel(proc::ground(8, 7))); // GROUND
    models.push_back(makeModel(proc::post())); // POST
    models.push_back(makeModel(proc::conifer(glm::vec3(0.08f, 0.36f, 0.13f)))); // tmavy jehlicnan
    models.push_back(makeModel(proc::conifer(glm::vec3(0.16f, 0.50f, 0.17f)))); // svetlejsi jehlicnan
    models.push_back(makeModel(proc::broadleaf(glm::vec3(0.20f, 0.52f, 0.14f)))); // listnac
    models.push_back(makeModel(proc::broadleaf(glm::vec3(0.34f, 0.62f, 0.20f)))); // svetly listnac
    models.push_back(makeModel(proc::bush(glm::vec3(0.12f, 0.42f, 0.14f)))); // tmavy ker
    models.push_back(makeModel(proc::bush(glm::vec3(0.26f, 0.55f, 0.18f)))); // svetly ker
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
    clockScene->add(models[CLOCK], clockProgram, placeY(0, 0, 0, 0.9f, PI));
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

void Application::setScene(int index)
{
    if (index >= 0 && index < static_cast<int>(scenes.size()))
    {
        activeScene = index;
        const Scene* scene = scenes[activeScene];
        camera.setPose(scene->getCameraEye(), scene->getCameraFi(), scene->getCameraAlpha());
        moveSpeed = scene->getMoveSpeed();
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


    const double now = glfwGetTime();
    const float step = moveSpeed * static_cast<float>(now - lastFrameTime);
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
    if (!scenes.empty()) setScene(activeScene);
    lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        processInput();

        glClearColor(0.035f, 0.045f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateClock();
        if (!scenes.empty())
            scenes[activeScene]->draw(static_cast<float>(glfwGetTime()));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
}