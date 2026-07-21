#include "Renderer.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <iostream>

namespace
{
// A vertex shader runs once for every vertex submitted by the mesh. Attribute
// locations 0 and 1 match the position and color layout configured by Mesh.
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
    // Values written to an "out" variable are smoothly interpolated across the
    // triangle before they reach the fragment shader.
    vertexColor = aColor;

    // Move the vertex from local space, through world and camera space, into
    // clip space. OpenGL uses gl_Position to assemble and rasterize triangles.
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
}
)";

// A fragment shader runs for every covered screen sample (roughly, every pixel
// of the triangle). Here it simply writes the interpolated vertex color.
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
    // Shader source code is compiled by the graphics driver at runtime.
    const unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Compilation errors are reported here instead of failing silently later
    // when the shader program is used for drawing.
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
    // Vertex and fragment shaders are compiled independently, then linked into
    // one program that represents the programmable part of our pipeline.
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

    // Linking copies the compiled shader code into the program, so the separate
    // shader objects are no longer needed after this point.
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
    // Keep the fragment closest to the camera when triangles overlap. Without
    // a depth buffer test, later triangles would always paint over earlier ones.
    glEnable(GL_DEPTH_TEST);
    return createResources();
}

bool Renderer::createResources()
{
    // These objects live on the GPU and only need to be created once. Each frame
    // can then reuse the same shader program and cube buffers.
    m_shaderProgram = createShaderProgram();
    if (m_shaderProgram == 0)
    {
        return false;
    }

    m_cube = Mesh::createCube();
    return m_cube.isValid();
}

void Renderer::beginFrame(int framebufferWidth, int framebufferHeight, const glm::mat4& view)
{
    // Clear both last frame's color and its depth information before drawing the
    // new frame. The clear color becomes the window background.
    glClearColor(0.08f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // The projection needs the framebuffer's aspect ratio so the cube does not
    // stretch when the window changes shape. Height can be zero while minimized.
    const float aspectRatio = framebufferHeight > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight) : 1.0f;

    // Projection adds perspective: distant geometry appears smaller. The last
    // two values are the near and far visible clipping distances.
    const glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    // Select the shader program, then upload this frame's transforms to its
    // uniform variables. GL_FALSE tells OpenGL not to transpose GLM's matrices.
    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uView"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uProjection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void Renderer::drawCube(const glm::mat4& model)
{
    // The model matrix changes for each object, while the view and projection
    // set by beginFrame are shared by every object in the frame.
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uModel"), 1, GL_FALSE, glm::value_ptr(model));

    // Mesh::draw binds the cube's vertex layout and asks OpenGL to draw its
    // indexed triangles with glDrawElements.
    m_cube.draw();
}

void Renderer::cleanup()
{
    // GPU resources must be released while the OpenGL context still exists.
    m_cube.cleanup();

    if (m_shaderProgram != 0)
    {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
}
