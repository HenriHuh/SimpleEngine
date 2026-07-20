#pragma once

class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool initialize();
    void render(int framebufferWidth, int framebufferHeight, float elapsedTime);
    void cleanup();

private:
    bool createBoxResources();

    unsigned int m_shaderProgram = 0;
    unsigned int m_vertexArray = 0;
    unsigned int m_vertexBuffer = 0;
};
