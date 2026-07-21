#include "Mesh.h"

#include <glad/gl.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace
{
constexpr std::array<Vertex, 24> CubeVertices = {
    // Front
    Vertex{{-0.5f, -0.5f,  0.5f}, {0.20f, 0.62f, 0.92f}},
    Vertex{{ 0.5f, -0.5f,  0.5f}, {0.20f, 0.62f, 0.92f}},
    Vertex{{ 0.5f,  0.5f,  0.5f}, {0.20f, 0.62f, 0.92f}},
    Vertex{{-0.5f,  0.5f,  0.5f}, {0.20f, 0.62f, 0.92f}},

    // Back
    Vertex{{ 0.5f, -0.5f, -0.5f}, {0.80f, 0.26f, 0.32f}},
    Vertex{{-0.5f, -0.5f, -0.5f}, {0.80f, 0.26f, 0.32f}},
    Vertex{{-0.5f,  0.5f, -0.5f}, {0.80f, 0.26f, 0.32f}},
    Vertex{{ 0.5f,  0.5f, -0.5f}, {0.80f, 0.26f, 0.32f}},

    // Left
    Vertex{{-0.5f, -0.5f, -0.5f}, {0.36f, 0.76f, 0.42f}},
    Vertex{{-0.5f, -0.5f,  0.5f}, {0.36f, 0.76f, 0.42f}},
    Vertex{{-0.5f,  0.5f,  0.5f}, {0.36f, 0.76f, 0.42f}},
    Vertex{{-0.5f,  0.5f, -0.5f}, {0.36f, 0.76f, 0.42f}},

    // Right
    Vertex{{0.5f, -0.5f,  0.5f}, {0.95f, 0.76f, 0.24f}},
    Vertex{{0.5f, -0.5f, -0.5f}, {0.95f, 0.76f, 0.24f}},
    Vertex{{0.5f,  0.5f, -0.5f}, {0.95f, 0.76f, 0.24f}},
    Vertex{{0.5f,  0.5f,  0.5f}, {0.95f, 0.76f, 0.24f}},

    // Bottom
    Vertex{{-0.5f, -0.5f, -0.5f}, {0.64f, 0.42f, 0.86f}},
    Vertex{{ 0.5f, -0.5f, -0.5f}, {0.64f, 0.42f, 0.86f}},
    Vertex{{ 0.5f, -0.5f,  0.5f}, {0.64f, 0.42f, 0.86f}},
    Vertex{{-0.5f, -0.5f,  0.5f}, {0.64f, 0.42f, 0.86f}},

    // Top
    Vertex{{-0.5f, 0.5f,  0.5f}, {0.96f, 0.50f, 0.22f}},
    Vertex{{ 0.5f, 0.5f,  0.5f}, {0.96f, 0.50f, 0.22f}},
    Vertex{{ 0.5f, 0.5f, -0.5f}, {0.96f, 0.50f, 0.22f}},
    Vertex{{-0.5f, 0.5f, -0.5f}, {0.96f, 0.50f, 0.22f}},
};

constexpr std::array<std::uint32_t, 36> CubeIndices = {
     0,  1,  2,  2,  3,  0,
     4,  5,  6,  6,  7,  4,
     8,  9, 10, 10, 11,  8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
};
}

Mesh::~Mesh()
{
    cleanup();
}

Mesh::Mesh(Mesh&& other) noexcept
    : m_vertexArray(std::exchange(other.m_vertexArray, 0)),
      m_vertexBuffer(std::exchange(other.m_vertexBuffer, 0)),
      m_indexBuffer(std::exchange(other.m_indexBuffer, 0)),
      m_indexCount(std::exchange(other.m_indexCount, 0))
{
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        cleanup();
        m_vertexArray = std::exchange(other.m_vertexArray, 0);
        m_vertexBuffer = std::exchange(other.m_vertexBuffer, 0);
        m_indexBuffer = std::exchange(other.m_indexBuffer, 0);
        m_indexCount = std::exchange(other.m_indexCount, 0);
    }

    return *this;
}

Mesh Mesh::createCube()
{
    Mesh mesh;

    glGenVertexArrays(1, &mesh.m_vertexArray);
    glGenBuffers(1, &mesh.m_vertexBuffer);
    glGenBuffers(1, &mesh.m_indexBuffer);

    glBindVertexArray(mesh.m_vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.m_vertexBuffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(CubeVertices.size() * sizeof(Vertex)),
        CubeVertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.m_indexBuffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(CubeIndices.size() * sizeof(std::uint32_t)),
        CubeIndices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh.m_indexCount = static_cast<int>(CubeIndices.size());
    return mesh;
}

void Mesh::draw() const
{
    glBindVertexArray(m_vertexArray);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

bool Mesh::isValid() const
{
    return m_vertexArray != 0 && m_vertexBuffer != 0 && m_indexBuffer != 0;
}

void Mesh::cleanup()
{
    if (m_indexBuffer != 0)
    {
        glDeleteBuffers(1, &m_indexBuffer);
        m_indexBuffer = 0;
    }

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

    m_indexCount = 0;
}
