#pragma once

#include "Mesh.h"

#include <filesystem>
#include <string>
#include <vector>

#include <glm/mat4x4.hpp>

struct ModelPart
{
    Mesh mesh;
    glm::mat4 localTransform{1.0f};
};

class Model
{
public:
    Model() = default;

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) noexcept = default;
    Model& operator=(Model&&) noexcept = default;

    bool loadFromFile(const std::filesystem::path& path);

    bool isValid() const;
    const std::vector<ModelPart>& getParts() const;
    const std::string& getLastError() const;

private:
    std::vector<ModelPart> m_parts;
    std::string m_lastError;
};
