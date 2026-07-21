#pragma once

#include "Camera.h"
#include "Transform.h"

#include <memory>

struct GLFWwindow;
class Renderer;

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    bool initialize();
    bool initializeWindow();
    bool initializeOpenGL();

    void processInput(float deltaTime);
    void processMouseInput();
    void update();
    void render();
    void cleanup();

    GLFWwindow* m_window = nullptr;
    std::unique_ptr<Renderer> m_renderer;
    Camera m_camera;
    Transform m_cubeTransform;
    float m_deltaTime = 0.0f;
    float m_lastFrameTime = 0.0f;
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_isFirstMouseInput = true;
};
