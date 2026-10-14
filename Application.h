#pragma once
#include "Camera.h"
#include "Model.h"
#include "Scene.h"
#include "ShaderProgram.h"
#include "Transform.h"
#include <vector>

class Application
{
public:
    explicit Application(bool broken);
    ~Application();

    void initialization();
    void createShaders();
    void createModels();
    void createScenes();
    void run();

    // volaji callbacky GLFW (mys, zmena velikosti okna)
    void onCursorMoved(double x, double y);
    void onFramebufferResized(int width, int height);

private:
    void processInput();
    void setScene(int index);
    void updateTitle();

    bool broken;
    GLFWwindow* window;
    ShaderProgram* basicProgram;
    ShaderProgram* redProgram;
    std::vector<Model*> models;
    std::vector<Scene*> scenes;
    int activeScene;

    CompositePtr triangleTransform;
    bool translateFirst;

    Camera camera;
    bool firstMouse;
    double lastCursorX;
    double lastCursorY;
    double lastFrameTime;
};