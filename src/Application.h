#pragma once

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

    void processInput();
    void update();
    void render();
    void cleanup();

    GLFWwindow* m_window = nullptr;
    std::unique_ptr<Renderer> m_renderer;
    float m_lastFrameTime = 0.0f;
    float m_elapsedTime = 0.0f;
};
