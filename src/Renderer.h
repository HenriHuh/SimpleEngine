#pragma once

#include "Mesh.h"

#include <glm/mat4x4.hpp>

class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool initialize();
    void render(
        int framebufferWidth,
        int framebufferHeight,
        const glm::mat4& model,
        const glm::mat4& view);
    void cleanup();

private:
    bool createResources();

    unsigned int m_shaderProgram = 0;
    Mesh m_cube;
};
