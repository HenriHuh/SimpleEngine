#pragma once

#include "Mesh.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Model;

class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool initialize();
    void beginFrame(int framebufferWidth, int framebufferHeight, const glm::mat4& view);
    void drawCube(const glm::mat4& model);
    void drawMesh(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color);
    void drawModel(const Model& model, const glm::mat4& transform, const glm::vec3& color);
    void drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color);
    void drawCrosshair(int framebufferWidth, int framebufferHeight);
    void cleanup();

private:
    bool createResources();

    unsigned int m_shaderProgram = 0;
    unsigned int m_crosshairShaderProgram = 0;
    unsigned int m_crosshairVertexArray = 0;
    unsigned int m_crosshairVertexBuffer = 0;
    unsigned int m_lineShaderProgram = 0;
    unsigned int m_lineVertexArray = 0;
    unsigned int m_lineVertexBuffer = 0;
    Mesh m_cube;
};
