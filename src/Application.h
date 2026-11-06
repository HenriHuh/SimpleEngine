#pragma once

#include "Game.h"

#include <memory>

struct GLFWwindow;
class Model;
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
    struct WindowDeleter
    {
        void operator()(GLFWwindow* window) const noexcept;
    };

    bool initialize();
    bool initializeWindow();
    bool initializeOpenGL();

    GameInput processInput();
    void processMouseInput(GameInput& input);
    void updateFrameTime();
    void updateWindowTitle();
    void render();
    void cleanup();

    std::unique_ptr<GLFWwindow, WindowDeleter> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Model> m_enemyModel;
    glm::mat4 m_enemyModelTransform{1.0f};
    Game m_game;
    float m_deltaTime = 0.0f;
    double m_lastFrameTime = 0.0;
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_isFirstMouseInput = true;
    bool m_wasRestartPressed = false;
};
