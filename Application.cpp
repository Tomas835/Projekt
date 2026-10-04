#include "Application.h"
#include "models/sphere.h"
#include "models/vyl0034_logo.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
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
        addTriangle(d, 0.0f, 0.55f, 0.0f,  -0.5f,-0.45f,0.0f,  0.5f,-0.45f,0.0f,
                    0.95f,0.25f,0.10f);
        return makeModel(d);
    }

    Model* createTreeModel()
    {
        std::vector<float> d;
        const float w = 0.12f;
        addTriangle(d, -w,-0.75f,0.0f,  w,-0.75f,0.0f,  w,0.0f,0.0f,
                    0.48f,0.25f,0.08f);
        addTriangle(d, -w,-0.75f,0.0f,  w,0.0f,0.0f, -w,0.0f,0.0f,
                    0.48f,0.25f,0.08f);
        addTriangle(d, 0.0f,0.72f,0.0f, -0.52f,-0.08f,0.0f, 0.52f,-0.08f,0.0f,
                    0.08f,0.48f,0.12f);
        addTriangle(d, 0.0f,0.42f,0.0f, -0.42f,-0.30f,0.0f, 0.42f,-0.30f,0.0f,
                    0.05f,0.38f,0.08f);
        return makeModel(d);
    }

    Model* createBushModel()
    {
        std::vector<float> d;
        addTriangle(d, -0.55f,-0.35f,0, 0.0f,0.45f,0, 0.55f,-0.35f,0,
                    0.10f,0.55f,0.12f);
        addTriangle(d, -0.45f,-0.30f,0.15f, 0.0f,0.38f,0.15f, 0.45f,-0.30f,0.15f,
                    0.16f,0.68f,0.18f);
        return makeModel(d);
    }

    Model* createSunModel()
    {
        std::vector<float> d;
        for (int i = 0; i < 8; ++i)
        {
            float a1 = (2.0f * PI * i) / 8.0f;
            float a2 = (2.0f * PI * (i + 1)) / 8.0f;
            addTriangle(d, 0,0,0, std::cos(a1),std::sin(a1),0, std::cos(a2),std::sin(a2),0,
                        1.0f,0.82f,0.08f);
        }
        return makeModel(d);
    }

    Model* createSignatureModel()
    {
        return new Model(vyl0034_logo, static_cast<int>(sizeof(vyl0034_logo) / sizeof(float)));
    }
}

Application::Application(bool b)
    : broken(b), window(nullptr), basicProgram(nullptr), redProgram(nullptr), activeScene(0) {}

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
}

void Application::createShaders()
{
    basicProgram = new ShaderProgram("shaders/basic.vert", "shaders/basic.frag");

    const char* redFrag = broken ? "shaders/broken.frag" : "shaders/red.frag";
    redProgram = new ShaderProgram("shaders/basic.vert", redFrag);
}

void Application::createModels()
{
    models.push_back(createTriangleModel()); 
    models.push_back(new Model(sphere, sizeof(sphere) / sizeof(float))); 
    models.push_back(createTreeModel()); 
    models.push_back(createBushModel()); 
    models.push_back(createSunModel()); 
    models.push_back(createSignatureModel()); 
}

void Application::createScenes()
{
	//Trojuhelnik
    Scene* triangle = new Scene("Trojuhelnik");
    triangle->add(models[0], basicProgram, 0, 0, 0, 0.9f, 0.0f);
    triangle->add(models[5], basicProgram, 0.68f, -0.78f, 0, 0.18f, 0.0f);
    scenes.push_back(triangle);

    //Koule
    Scene* ball = new Scene("Kulicka");
    ball->add(models[1], redProgram, 0, 0, 0, 0.75f, 0.0f);
    ball->add(models[5], basicProgram, 0.68f, -0.78f, 0, 0.18f, 0.0f);
    scenes.push_back(ball);

    //Les
    Scene* forest = new Scene("Les");
    const float treePositions[16][2] = {
        {-0.85f,-0.65f},{-0.58f,-0.58f},{-0.28f,-0.66f},{0.02f,-0.58f},
        {0.32f,-0.68f},{0.62f,-0.58f},{0.86f,-0.68f},{-0.72f,-0.20f},
        {-0.42f,-0.12f},{-0.10f,-0.22f},{0.22f,-0.10f},{0.52f,-0.20f},
        {0.80f,-0.12f},{-0.20f,0.22f},{0.38f,0.30f},{-0.72f,0.28f}
    };
    for (int i = 0; i < 16; ++i)
    {
        float scale = 0.22f + (i % 3) * 0.025f;
        forest->add(models[2], basicProgram, treePositions[i][0], treePositions[i][1], 0, scale,
                    (i % 2 == 0 ? -0.04f : 0.04f));
    }

    const float bushPositions[16][2] = {
        {-0.92f,-0.92f},{-0.70f,-0.88f},{-0.47f,-0.92f},{-0.22f,-0.86f},
        {0.05f,-0.90f},{0.30f,-0.86f},{0.56f,-0.91f},{0.82f,-0.88f},
        {-0.60f,-0.40f},{-0.30f,-0.42f},{0.00f,-0.38f},{0.30f,-0.43f},
        {0.58f,-0.38f},{-0.88f,0.02f},{0.02f,0.00f},{0.72f,0.02f}
    };
    for (int i = 0; i < 16; ++i)
    {
        forest->add(models[3], basicProgram, bushPositions[i][0], bushPositions[i][1], 0, 0.10f + (i % 2) * 0.015f, 0);
    }
    forest->add(models[4], basicProgram, 0.72f, 0.70f, 0, 0.16f, 0);
    forest->add(models[5], basicProgram, 0.72f, -0.78f, 0, 0.16f, 0);
    scenes.push_back(forest);

    //Login
    Scene* signature = new Scene("Vlastni login");
    signature->add(models[5], basicProgram, 0, 0, 0, 0.75f, 0.0f);
    signature->add(models[1], redProgram, 0, -0.68f, 0, 0.14f, 0.0f);
    scenes.push_back(signature);
}

void Application::setScene(int index)
{
    if (index >= 0 && index < static_cast<int>(scenes.size()))
    {
        activeScene = index;
        std::cout << "Aktivni scena: " << activeScene + 1 << " - "
                  << scenes[activeScene]->getName() << std::endl;
    }
}

void Application::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    static bool previous[4] = { false, false, false, false };
    const int keys[4] = { GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4 };
    for (int i = 0; i < 4; ++i)
    {
        bool pressed = glfwGetKey(window, keys[i]) == GLFW_PRESS;
        if (pressed && !previous[i]) setScene(i);
        previous[i] = pressed;
    }
}

void Application::run()
{
    while (!glfwWindowShouldClose(window))
    {
        processInput();

        glClearColor(0.035f, 0.045f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!scenes.empty())
            scenes[activeScene]->draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
}
