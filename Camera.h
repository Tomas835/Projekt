#pragma once
#include <glm/glm.hpp>
#include <vector>

class ShaderProgram; // dopredna deklarace (z duvodu krizoveho odkazu Camera <-> ShaderProgram)

// Kamera = subjekt navrhoveho vzoru Observer.
// Shader programy se u kamery zaregistruji (attach) a pri kazde zmene kamery
// (pohled, pozice, pomer stran okna) jim kamera posle nove matice.
class Camera
{
public:
    Camera();

    void attach(ShaderProgram* program);
    void detach(ShaderProgram* program);

    glm::mat4 getViewMatrix() const;       // lookAt(eye, eye + target, up)
    glm::mat4 getProjectionMatrix() const; // perspective(fov, aspect, zNear, zFar)

    // Otoceni pohledu (uhly v radianech): fi = doleva/doprava, alpha = nahoru/dolu.
    void rotate(float deltaFi, float deltaAlpha);
    // Posun kamery: dopredu/dozadu ve smeru pohledu a do stran.
    void move(float forward, float right);
    void setAspect(float aspect);

private:
    void updateTarget();
    void notify() const;

    glm::vec3 eye;    // pozice kamery
    glm::vec3 target; // smer pohledu (jednotkovy vektor)
    glm::vec3 up;
    float alpha;      // uhel od osy +Y (omezen, aby se kamera nepreklopila)
    float fi;         // uhel v rovine XZ

    float fov;        // zorny uhel v radianech
    float aspect;     // sirka / vyska okna
    float zNear;
    float zFar;

    std::vector<ShaderProgram*> observers;
};