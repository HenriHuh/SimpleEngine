#pragma once

#include <glm/vec3.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

class Mesh
{
public:
    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    static Mesh createCube();

    void draw() const;
    bool isValid() const;
    void cleanup();

private:
    unsigned int m_vertexArray = 0;
    unsigned int m_vertexBuffer = 0;
    unsigned int m_indexBuffer = 0;
    int m_indexCount = 0;
};
