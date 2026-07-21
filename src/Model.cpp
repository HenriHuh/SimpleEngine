#include "Model.h"

#include <tiny_gltf.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <utility>

namespace
{
struct AccessorView
{
    const unsigned char* data = nullptr;
    std::size_t stride = 0;
    std::size_t count = 0;
    int componentType = 0;
    int componentCount = 0;
    bool normalized = false;
};

template<typename T>
T readValue(const unsigned char* data)
{
    T value{};
    std::memcpy(&value, data, sizeof(T));
    return value;
}

float readFloatComponent(const unsigned char* data, int componentType, bool normalized)
{
    switch (componentType)
    {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
    {
        const auto value = readValue<std::int8_t>(data);
        return normalized ? std::max(static_cast<float>(value) / 127.0f, -1.0f) : static_cast<float>(value);
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    {
        const auto value = readValue<std::uint8_t>(data);
        return normalized ? static_cast<float>(value) / 255.0f : static_cast<float>(value);
    }
    case TINYGLTF_COMPONENT_TYPE_SHORT:
    {
        const auto value = readValue<std::int16_t>(data);
        return normalized ? std::max(static_cast<float>(value) / 32767.0f, -1.0f) : static_cast<float>(value);
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    {
        const auto value = readValue<std::uint16_t>(data);
        return normalized ? static_cast<float>(value) / 65535.0f : static_cast<float>(value);
    }
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    {
        const auto value = readValue<std::uint32_t>(data);
        return normalized
            ? static_cast<float>(static_cast<double>(value) / 4294967295.0)
            : static_cast<float>(value);
    }
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        return readValue<float>(data);
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
        return static_cast<float>(readValue<double>(data));
    default:
        return 0.0f;
    }
}

bool makeAccessorView(
    const tinygltf::Model& model,
    int accessorIndex,
    int expectedType,
    AccessorView& result,
    std::string& error)
{
    if (accessorIndex < 0 || accessorIndex >= static_cast<int>(model.accessors.size()))
    {
        error = "Model contains an invalid accessor index.";
        return false;
    }

    const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
    if (accessor.sparse.isSparse)
    {
        error = "Sparse glTF accessors are not supported yet.";
        return false;
    }
    if (accessor.type != expectedType)
    {
        error = "A glTF accessor has an unexpected data type.";
        return false;
    }
    if (accessor.bufferView < 0 || accessor.bufferView >= static_cast<int>(model.bufferViews.size()))
    {
        error = "A glTF accessor has no valid buffer view.";
        return false;
    }

    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    if (bufferView.buffer < 0 || bufferView.buffer >= static_cast<int>(model.buffers.size()))
    {
        error = "A glTF buffer view references an invalid buffer.";
        return false;
    }

    const int componentSize = tinygltf::GetComponentSizeInBytes(static_cast<std::uint32_t>(accessor.componentType));
    const int componentCount = tinygltf::GetNumComponentsInType(static_cast<std::uint32_t>(accessor.type));
    const int byteStride = accessor.ByteStride(bufferView);
    if (componentSize <= 0 || componentCount <= 0 || byteStride <= 0)
    {
        error = "A glTF accessor has an unsupported component layout.";
        return false;
    }

    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
    const std::size_t start = bufferView.byteOffset + accessor.byteOffset;
    const std::size_t viewEnd = bufferView.byteOffset + bufferView.byteLength;
    const std::size_t elementSize = static_cast<std::size_t>(componentSize * componentCount);
    const std::size_t stride = static_cast<std::size_t>(byteStride);
    if (bufferView.byteOffset > buffer.data.size()
        || bufferView.byteLength > buffer.data.size() - bufferView.byteOffset
        || accessor.byteOffset > bufferView.byteLength
        || start > viewEnd
        || stride < elementSize)
    {
        error = "A glTF accessor points outside its buffer.";
        return false;
    }

    if (accessor.count > 0)
    {
        const std::size_t remainingBytes = viewEnd - start;
        if (remainingBytes < elementSize
            || accessor.count - 1 > (remainingBytes - elementSize) / stride)
        {
            error = "A glTF accessor extends past the end of its buffer.";
            return false;
        }
    }

    result.data = buffer.data.data() + start;
    result.stride = stride;
    result.count = accessor.count;
    result.componentType = accessor.componentType;
    result.componentCount = componentCount;
    result.normalized = accessor.normalized;
    return true;
}

bool readFloatAccessor(
    const tinygltf::Model& model,
    int accessorIndex,
    int expectedType,
    std::vector<float>& values,
    std::string& error)
{
    AccessorView view;
    if (!makeAccessorView(model, accessorIndex, expectedType, view, error))
    {
        return false;
    }

    const int componentSize = tinygltf::GetComponentSizeInBytes(static_cast<std::uint32_t>(view.componentType));
    values.resize(view.count * static_cast<std::size_t>(view.componentCount));

    for (std::size_t element = 0; element < view.count; ++element)
    {
        const unsigned char* source = view.data + element * view.stride;
        for (int component = 0; component < view.componentCount; ++component)
        {
            values[element * view.componentCount + component] = readFloatComponent(
                source + component * componentSize,
                view.componentType,
                view.normalized);
        }
    }

    return true;
}

bool readIndices(
    const tinygltf::Model& model,
    int accessorIndex,
    std::vector<std::uint32_t>& indices,
    std::string& error)
{
    AccessorView view;
    if (!makeAccessorView(model, accessorIndex, TINYGLTF_TYPE_SCALAR, view, error))
    {
        return false;
    }

    indices.resize(view.count);
    for (std::size_t index = 0; index < view.count; ++index)
    {
        const unsigned char* source = view.data + index * view.stride;
        switch (view.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            indices[index] = readValue<std::uint8_t>(source);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            indices[index] = readValue<std::uint16_t>(source);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            indices[index] = readValue<std::uint32_t>(source);
            break;
        default:
            error = "A glTF index accessor uses an unsupported component type.";
            return false;
        }
    }

    return true;
}

glm::mat4 getNodeTransform(const tinygltf::Node& node)
{
    if (node.matrix.size() == 16)
    {
        glm::mat4 transform(1.0f);
        for (int column = 0; column < 4; ++column)
        {
            for (int row = 0; row < 4; ++row)
            {
                transform[column][row] = static_cast<float>(node.matrix[column * 4 + row]);
            }
        }
        return transform;
    }

    glm::vec3 translation(0.0f);
    glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale(1.0f);

    if (node.translation.size() == 3)
    {
        translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
    }
    if (node.rotation.size() == 4)
    {
        rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
    }
    if (node.scale.size() == 3)
    {
        scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
    }

    return glm::translate(glm::mat4(1.0f), translation)
        * glm::mat4_cast(rotation)
        * glm::scale(glm::mat4(1.0f), scale);
}

bool loadPrimitive(
    const tinygltf::Model& source,
    const tinygltf::Primitive& primitive,
    const glm::mat4& transform,
    std::vector<ModelPart>& parts,
    std::string& error)
{
    if (primitive.mode != -1 && primitive.mode != TINYGLTF_MODE_TRIANGLES)
    {
        error = "Only triangle-list glTF primitives are supported.";
        return false;
    }

    const auto positionAttribute = primitive.attributes.find("POSITION");
    if (positionAttribute == primitive.attributes.end())
    {
        error = "A glTF mesh primitive is missing POSITION data.";
        return false;
    }

    std::vector<float> positions;
    if (!readFloatAccessor(source, positionAttribute->second, TINYGLTF_TYPE_VEC3, positions, error))
    {
        return false;
    }

    const std::size_t vertexCount = positions.size() / 3;
    std::vector<Vertex> vertices(vertexCount);
    for (std::size_t index = 0; index < vertexCount; ++index)
    {
        vertices[index].position = glm::vec3(
            positions[index * 3],
            positions[index * 3 + 1],
            positions[index * 3 + 2]);
        vertices[index].normal = glm::vec3(0.0f, 0.0f, 1.0f);
        vertices[index].textureCoordinates = glm::vec2(0.0f);
    }

    const auto normalAttribute = primitive.attributes.find("NORMAL");
    if (normalAttribute != primitive.attributes.end())
    {
        std::vector<float> normals;
        if (!readFloatAccessor(source, normalAttribute->second, TINYGLTF_TYPE_VEC3, normals, error)
            || normals.size() / 3 != vertexCount)
        {
            error = error.empty() ? "A glTF NORMAL accessor has the wrong vertex count." : error;
            return false;
        }

        for (std::size_t index = 0; index < vertexCount; ++index)
        {
            vertices[index].normal = glm::vec3(
                normals[index * 3],
                normals[index * 3 + 1],
                normals[index * 3 + 2]);
        }
    }

    const auto textureAttribute = primitive.attributes.find("TEXCOORD_0");
    if (textureAttribute != primitive.attributes.end())
    {
        std::vector<float> textureCoordinates;
        if (!readFloatAccessor(source, textureAttribute->second, TINYGLTF_TYPE_VEC2, textureCoordinates, error)
            || textureCoordinates.size() / 2 != vertexCount)
        {
            error = error.empty() ? "A glTF TEXCOORD_0 accessor has the wrong vertex count." : error;
            return false;
        }

        for (std::size_t index = 0; index < vertexCount; ++index)
        {
            vertices[index].textureCoordinates = glm::vec2(
                textureCoordinates[index * 2],
                textureCoordinates[index * 2 + 1]);
        }
    }

    std::vector<std::uint32_t> indices;
    if (primitive.indices >= 0)
    {
        if (!readIndices(source, primitive.indices, indices, error))
        {
            return false;
        }
    }
    else
    {
        if (vertexCount > std::numeric_limits<std::uint32_t>::max())
        {
            error = "A non-indexed glTF primitive has too many vertices.";
            return false;
        }

        indices.resize(vertexCount);
        for (std::size_t index = 0; index < vertexCount; ++index)
        {
            indices[index] = static_cast<std::uint32_t>(index);
        }
    }

    if (indices.size() % 3 != 0)
    {
        error = "A triangle-list glTF primitive has an incomplete triangle.";
        return false;
    }
    if (std::any_of(indices.begin(), indices.end(), [vertexCount](std::uint32_t index)
        {
            return index >= vertexCount;
        }))
    {
        error = "A glTF primitive contains an index outside its vertex data.";
        return false;
    }

    Mesh mesh = Mesh::create(vertices, indices);
    if (!mesh.isValid())
    {
        error = "Failed to create OpenGL buffers for a glTF mesh.";
        return false;
    }

    parts.push_back(ModelPart{std::move(mesh), transform});
    return true;
}

bool loadMesh(
    const tinygltf::Model& source,
    int meshIndex,
    const glm::mat4& transform,
    std::vector<ModelPart>& parts,
    std::string& error)
{
    if (meshIndex < 0 || meshIndex >= static_cast<int>(source.meshes.size()))
    {
        error = "A glTF node references an invalid mesh.";
        return false;
    }

    for (const tinygltf::Primitive& primitive : source.meshes[meshIndex].primitives)
    {
        if (!loadPrimitive(source, primitive, transform, parts, error))
        {
            return false;
        }
    }
    return true;
}

bool loadNode(
    const tinygltf::Model& source,
    int nodeIndex,
    const glm::mat4& parentTransform,
    std::vector<bool>& activeNodes,
    std::vector<ModelPart>& parts,
    std::string& error)
{
    if (nodeIndex < 0 || nodeIndex >= static_cast<int>(source.nodes.size()))
    {
        error = "A glTF scene references an invalid node.";
        return false;
    }
    if (activeNodes[nodeIndex])
    {
        error = "A cycle was found in the glTF node hierarchy.";
        return false;
    }

    activeNodes[nodeIndex] = true;
    const tinygltf::Node& node = source.nodes[nodeIndex];
    const glm::mat4 transform = parentTransform * getNodeTransform(node);

    if (node.mesh >= 0 && !loadMesh(source, node.mesh, transform, parts, error))
    {
        return false;
    }

    for (int childIndex : node.children)
    {
        if (!loadNode(source, childIndex, transform, activeNodes, parts, error))
        {
            return false;
        }
    }

    activeNodes[nodeIndex] = false;
    return true;
}
}

bool Model::loadFromFile(const std::filesystem::path& path)
{
    m_parts.clear();
    m_lastError.clear();

    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

    tinygltf::TinyGLTF loader;
    tinygltf::Model source;
    std::string warning;
    std::string error;
    bool loaded = false;

    if (extension == ".glb")
    {
        loaded = loader.LoadBinaryFromFile(&source, &error, &warning, path.string());
    }
    else if (extension == ".gltf")
    {
        loaded = loader.LoadASCIIFromFile(&source, &error, &warning, path.string());
    }
    else
    {
        m_lastError = "Model must be a .gltf or .glb file.";
        return false;
    }

    if (!warning.empty())
    {
        std::cerr << "TinyGLTF warning for " << path.string() << ": " << warning << '\n';
    }
    if (!loaded)
    {
        m_lastError = error.empty() ? "TinyGLTF could not load the model." : error;
        return false;
    }

    std::vector<bool> activeNodes(source.nodes.size(), false);
    if (!source.scenes.empty())
    {
        const int sceneIndex = source.defaultScene >= 0 ? source.defaultScene : 0;
        if (sceneIndex < 0 || sceneIndex >= static_cast<int>(source.scenes.size()))
        {
            m_lastError = "The glTF default scene index is invalid.";
            return false;
        }

        for (int nodeIndex : source.scenes[sceneIndex].nodes)
        {
            if (!loadNode(source, nodeIndex, glm::mat4(1.0f), activeNodes, m_parts, m_lastError))
            {
                m_parts.clear();
                return false;
            }
        }
    }
    else
    {
        for (int meshIndex = 0; meshIndex < static_cast<int>(source.meshes.size()); ++meshIndex)
        {
            if (!loadMesh(source, meshIndex, glm::mat4(1.0f), m_parts, m_lastError))
            {
                m_parts.clear();
                return false;
            }
        }
    }

    if (m_parts.empty())
    {
        m_lastError = "The glTF file contains no supported triangle meshes.";
        return false;
    }

    return true;
}

bool Model::isValid() const
{
    return !m_parts.empty();
}

const std::vector<ModelPart>& Model::getParts() const
{
    return m_parts;
}

const std::string& Model::getLastError() const
{
    return m_lastError;
}
