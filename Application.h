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
    void applyLight(const Scene& scene, float time);   // time is needed for the moving light (firefly)

    bool broken;
    GLFWwindow* window;
    ShaderProgram* basicProgram;
    ShaderProgram* redProgram;
    ShaderProgram* clockProgram;
    ShaderProgram* constantProgram;   // ambient
    ShaderProgram* lambertProgram;    // ambient + diffuse
    ShaderProgram* phongProgram;      // + specular (reflect vector)
    ShaderProgram* blinnProgram;      // + specular (half-vector)
    ShaderProgram* phongWrongNormalsProgram; // Phong + WRONG normal conversion (mat3(M) * n)
    ShaderProgram* phongLeakProgram;         // Phong without clipping the highlight on the far side (shininess = 1)
    ShaderProgram* phongClipProgram;         // Phong with the highlight clipped on the far side (shininess = 1)
    ShaderProgram* nightMatteProgram;        // night forest, matte material (ground, bushes)
    ShaderProgram* nightTreeProgram;         // night forest, slightly shinier material (trees)
    std::vector<ShaderProgram*> litPrograms; // all programs that use lighting (receive the light uniforms)
    std::vector<Model*> models;
    std::vector<Scene*> scenes;
    int activeScene;

    CompositePtr triangleTransform;
    bool translateFirst;

    bool resizeCorrect;               // R key: correct / wrong viewport and projection update on window resize
    int framebufferWidth;             // last known framebuffer size
    int framebufferHeight;

    Camera camera;
    bool lookActive;
    float moveSpeed;
    bool firstMouse;
    double lastCursorX;
    double lastCursorY;
    double lastFrameTime;
};