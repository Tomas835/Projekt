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


    void onCursorMoved(double x, double y);
    void onMouseButton(int button, int action);
    void onFramebufferResized(int width, int height);

private:
    void processInput();
    void setScene(int index);
    void updateTitle();
    void updateClock();

    bool broken;
    GLFWwindow* window;
    ShaderProgram* basicProgram;
    ShaderProgram* redProgram;
    ShaderProgram* clockProgram;
    std::vector<Model*> models;
    std::vector<Scene*> scenes;
    int activeScene;

    CompositePtr triangleTransform;
    bool translateFirst;

    Camera camera;
    bool lookActive;   
    float moveSpeed;
    bool firstMouse;
    double lastCursorX;
    double lastCursorY;
    double lastFrameTime;
};