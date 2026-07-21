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
    bool initialize();
    bool initializeWindow();
    bool initializeOpenGL();

    GameInput processInput();
    void processMouseInput(GameInput& input);
    void updateFrameTime();
    void render();
    void cleanup();

    GLFWwindow* m_window = nullptr;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Model> m_enemyModel;
    Game m_game;
    float m_deltaTime = 0.0f;
    float m_lastFrameTime = 0.0f;
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_isFirstMouseInput = true;
};
