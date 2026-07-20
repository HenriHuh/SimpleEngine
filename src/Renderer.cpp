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

constexpr std::array<float, 216> BoxVertices = {
    // positions          // colors
    -0.5f, -0.5f, -0.5f,  0.80f, 0.26f, 0.32f,
     0.5f, -0.5f, -0.5f,  0.80f, 0.26f, 0.32f,
     0.5f,  0.5f, -0.5f,  0.80f, 0.26f, 0.32f,
     0.5f,  0.5f, -0.5f,  0.80f, 0.26f, 0.32f,
    -0.5f,  0.5f, -0.5f,  0.80f, 0.26f, 0.32f,
    -0.5f, -0.5f, -0.5f,  0.80f, 0.26f, 0.32f,

    -0.5f, -0.5f,  0.5f,  0.20f, 0.62f, 0.92f,
     0.5f, -0.5f,  0.5f,  0.20f, 0.62f, 0.92f,
     0.5f,  0.5f,  0.5f,  0.20f, 0.62f, 0.92f,
     0.5f,  0.5f,  0.5f,  0.20f, 0.62f, 0.92f,
    -0.5f,  0.5f,  0.5f,  0.20f, 0.62f, 0.92f,
    -0.5f, -0.5f,  0.5f,  0.20f, 0.62f, 0.92f,

    -0.5f,  0.5f,  0.5f,  0.36f, 0.76f, 0.42f,
    -0.5f,  0.5f, -0.5f,  0.36f, 0.76f, 0.42f,
    -0.5f, -0.5f, -0.5f,  0.36f, 0.76f, 0.42f,
    -0.5f, -0.5f, -0.5f,  0.36f, 0.76f, 0.42f,
    -0.5f, -0.5f,  0.5f,  0.36f, 0.76f, 0.42f,
    -0.5f,  0.5f,  0.5f,  0.36f, 0.76f, 0.42f,

     0.5f,  0.5f,  0.5f,  0.95f, 0.76f, 0.24f,
     0.5f,  0.5f, -0.5f,  0.95f, 0.76f, 0.24f,
     0.5f, -0.5f, -0.5f,  0.95f, 0.76f, 0.24f,
     0.5f, -0.5f, -0.5f,  0.95f, 0.76f, 0.24f,
     0.5f, -0.5f,  0.5f,  0.95f, 0.76f, 0.24f,
     0.5f,  0.5f,  0.5f,  0.95f, 0.76f, 0.24f,

    -0.5f, -0.5f, -0.5f,  0.64f, 0.42f, 0.86f,
     0.5f, -0.5f, -0.5f,  0.64f, 0.42f, 0.86f,
     0.5f, -0.5f,  0.5f,  0.64f, 0.42f, 0.86f,
     0.5f, -0.5f,  0.5f,  0.64f, 0.42f, 0.86f,
    -0.5f, -0.5f,  0.5f,  0.64f, 0.42f, 0.86f,
    -0.5f, -0.5f, -0.5f,  0.64f, 0.42f, 0.86f,

    -0.5f,  0.5f, -0.5f,  0.96f, 0.50f, 0.22f,
     0.5f,  0.5f, -0.5f,  0.96f, 0.50f, 0.22f,
     0.5f,  0.5f,  0.5f,  0.96f, 0.50f, 0.22f,
     0.5f,  0.5f,  0.5f,  0.96f, 0.50f, 0.22f,
    -0.5f,  0.5f,  0.5f,  0.96f, 0.50f, 0.22f,
    -0.5f,  0.5f, -0.5f,  0.96f, 0.50f, 0.22f,
};

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
    return createBoxResources();
}

bool Renderer::createBoxResources()
{
    m_shaderProgram = createShaderProgram();
    if (m_shaderProgram == 0)
    {
        return false;
    }

    glGenVertexArrays(1, &m_vertexArray);
    glGenBuffers(1, &m_vertexBuffer);

    glBindVertexArray(m_vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(BoxVertices.size() * sizeof(float)), BoxVertices.data(), GL_STATIC_DRAW);

    constexpr int stride = 6 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void Renderer::render(int framebufferWidth, int framebufferHeight)
{
    glClearColor(0.08f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspectRatio = framebufferHeight > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight) : 1.0f;

    const glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(-22.0f), glm::vec3(0.4f, 1.0f, 0.0f));
    const glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    const glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uView"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uProjection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(m_vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void Renderer::cleanup()
{
    if (m_vertexBuffer != 0)
    {
        glDeleteBuffers(1, &m_vertexBuffer);
        m_vertexBuffer = 0;
    }

    if (m_vertexArray != 0)
    {
        glDeleteVertexArrays(1, &m_vertexArray);
        m_vertexArray = 0;
    }

    if (m_shaderProgram != 0)
    {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
}
