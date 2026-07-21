#include "Renderer.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <iostream>

namespace
{
constexpr const char* VertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    vertexColor = aColor;
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
)";

constexpr const char* FragmentShaderSource = R"(
#version 330 core

in vec3 vertexColor;

out vec4 fragColor;

void main()
{
    fragColor = vec4(vertexColor, 1.0);
}
)";

unsigned int compileShader(unsigned int shaderType, const char* source)
{
    const unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        std::array<char, 1024> infoLog{};
        glGetShaderInfoLog(shader, static_cast<int>(infoLog.size()), nullptr, infoLog.data());
        std::cerr << "Shader compilation failed:\n" << infoLog.data() << '\n';
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

unsigned int createShaderProgram()
{
    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, VertexShaderSource);
    if (vertexShader == 0)
    {
        return 0;
    }

    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, FragmentShaderSource);
    if (fragmentShader == 0)
    {
        glDeleteShader(vertexShader);
        return 0;
    }

    const unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int success = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        std::array<char, 1024> infoLog{};
        glGetProgramInfoLog(shaderProgram, static_cast<int>(infoLog.size()), nullptr, infoLog.data());
        std::cerr << "Shader linking failed:\n" << infoLog.data() << '\n';
        glDeleteProgram(shaderProgram);
        return 0;
    }

    return shaderProgram;
}
}

Renderer::~Renderer()
{
    cleanup();
}

bool Renderer::initialize()
{
    glEnable(GL_DEPTH_TEST);
    return createResources();
}

bool Renderer::createResources()
{
    m_shaderProgram = createShaderProgram();
    if (m_shaderProgram == 0)
    {
        return false;
    }

    m_cube = Mesh::createCube();
    return m_cube.isValid();
}

void Renderer::render(int framebufferWidth, int framebufferHeight, float elapsedTime)
{
    glClearColor(0.08f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspectRatio = framebufferHeight > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight) : 1.0f;

    const glm::mat4 model = glm::rotate(glm::mat4(1.0f), elapsedTime, glm::vec3(0.4f, 1.0f, 0.0f));
    const glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    const glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uView"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uProjection"), 1, GL_FALSE, glm::value_ptr(projection));

    m_cube.draw();
}

void Renderer::cleanup()
{
    m_cube.cleanup();

    if (m_shaderProgram != 0)
    {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
}
