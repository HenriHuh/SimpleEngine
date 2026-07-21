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
    Vertex{{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
    Vertex{{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
    Vertex{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
    Vertex{{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},

    // Back
    Vertex{{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
    Vertex{{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
    Vertex{{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
    Vertex{{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},

    // Left
    Vertex{{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
    Vertex{{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
    Vertex{{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
    Vertex{{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},

    // Right
    Vertex{{0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
    Vertex{{0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
    Vertex{{0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
    Vertex{{0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},

    // Bottom
    Vertex{{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
    Vertex{{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
    Vertex{{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
    Vertex{{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},

    // Top
    Vertex{{-0.5f, 0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 0.0f}},
    Vertex{{ 0.5f, 0.5f,  0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 0.0f}},
    Vertex{{ 0.5f, 0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {1.0f, 1.0f}},
    Vertex{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f,  0.0f}, {0.0f, 1.0f}},
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

Mesh Mesh::create(const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices)
{
    Mesh mesh;

    if (vertices.empty() || indices.empty())
    {
        return mesh;
    }

    glGenVertexArrays(1, &mesh.m_vertexArray);
    glGenBuffers(1, &mesh.m_vertexBuffer);
    glGenBuffers(1, &mesh.m_indexBuffer);

    glBindVertexArray(mesh.m_vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.m_vertexBuffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.m_indexBuffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)),
        indices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, textureCoordinates)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh.m_indexCount = static_cast<int>(indices.size());
    return mesh;
}

Mesh Mesh::createCube()
{
    return create(
        std::vector<Vertex>(CubeVertices.begin(), CubeVertices.end()),
        std::vector<std::uint32_t>(CubeIndices.begin(), CubeIndices.end()));
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
