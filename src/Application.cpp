#define GLFW_INCLUDE_NONE

#include "Application.h"

#include "Model.h"
#include "Renderer.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <filesystem>
#include <iostream>

namespace
{
constexpr int WindowWidth = 1280;
constexpr int WindowHeight = 720;

void onFramebufferResize(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}
}

Application::Application() = default;

Application::~Application()
{
    cleanup();
}

int Application::run()
{
    if (!initialize())
    {
        return 1;
    }

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        updateFrameTime();
        const GameInput input = processInput();
        m_game.update(m_deltaTime, input);
        render();

        glfwSwapBuffers(m_window);
    }

    return 0;
}

bool Application::initialize()
{
    return initializeWindow() && initializeOpenGL();
}

bool Application::initializeWindow()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW.\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(WindowWidth, WindowHeight, "SimpleEngine", nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Failed to create window.\n";
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, onFramebufferResize);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwSwapInterval(1);

    return true;
}

bool Application::initializeOpenGL()
{
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to load OpenGL functions.\n";
        return false;
    }

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    m_renderer = std::make_unique<Renderer>();
    m_lastFrameTime = static_cast<float>(glfwGetTime());
    if (!m_renderer->initialize())
    {
        return false;
    }

    const std::filesystem::path enemyModelPath =
        std::filesystem::path(SIMPLE_ENGINE_ASSET_DIR) / "models" / "ship.glb";

    m_enemyModel = std::make_unique<Model>();
    if (!m_enemyModel->loadFromFile(enemyModelPath))
    {
        std::cerr << "Could not load enemy model at " << enemyModelPath << ": "
                  << m_enemyModel->getLastError()
                  << " Falling back to cubes.\n";
        m_enemyModel.reset();
    }

    return true;
}

GameInput Application::processInput()
{
    GameInput input;

    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    {
        input.moveForward += 1.0f;
    }
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    {
        input.moveForward -= 1.0f;
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    {
        input.moveRight += 1.0f;
    }
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    {
        input.moveRight -= 1.0f;
    }
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        input.moveUp += 1.0f;
    }
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        input.moveUp -= 1.0f;
    }

    input.fireRaycast = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    input.fireProjectile = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    processMouseInput(input);
    return input;
}

void Application::processMouseInput(GameInput& input)
{
    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(m_window, &mouseX, &mouseY);

    // The first position only establishes a starting point. Treating it as
    // movement would make the camera jump when the application starts.
    if (m_isFirstMouseInput)
    {
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;
        m_isFirstMouseInput = false;
        return;
    }

    const float horizontalOffset = static_cast<float>(mouseX - m_lastMouseX);
    const float verticalOffset = static_cast<float>(m_lastMouseY - mouseY);
    m_lastMouseX = mouseX;
    m_lastMouseY = mouseY;

    // GLFW's Y coordinates increase downward, so the subtraction above is
    // reversed to make moving the mouse upward look upward.
    input.lookHorizontal = horizontalOffset;
    input.lookVertical = verticalOffset;
}

void Application::updateFrameTime()
{
    const float currentFrameTime = static_cast<float>(glfwGetTime());
    m_deltaTime = currentFrameTime - m_lastFrameTime;
    m_lastFrameTime = currentFrameTime;
}

void Application::render()
{
    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

    m_renderer->beginFrame(framebufferWidth, framebufferHeight, m_game.getCamera().getViewMatrix());

    for (const Enemy& enemy : m_game.getEnemies())
    {
        if (m_enemyModel)
        {
            // The Blender placeholder is larger than the old unit cube, so
            // keep its rendered size close to the existing collision radius.
            const glm::mat4 modelTransform = glm::scale(
                enemy.transform.getMatrix(),
                glm::vec3(0.25f));
            m_renderer->drawModel(*m_enemyModel, modelTransform, glm::vec3(0.25f, 0.65f, 0.9f));
        }
        else
        {
            m_renderer->drawCube(enemy.transform.getMatrix());
        }
    }

    for (const Projectile& projectile : m_game.getProjectiles())
    {
        m_renderer->drawCube(projectile.transform.getMatrix());
    }

    for (const RaycastTracer& tracer : m_game.getRaycastTracers())
    {
        const float opacity = tracer.remainingLifetime / tracer.lifetime;
        m_renderer->drawLine(tracer.start, tracer.end, glm::vec4(0.25f, 0.9f, 1.0f, opacity));
    }

    m_renderer->drawCrosshair(framebufferWidth, framebufferHeight);
}

void Application::cleanup()
{
    // Models own OpenGL buffers and must be destroyed while the context is alive.
    m_enemyModel.reset();
    m_renderer.reset();

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    glfwTerminate();
}
