#ifndef GLTF_SCENE_HPP
#define GLTF_SCENE_HPP
#include "ResourceUploadHeap.hpp"
#include "VkBufferPool.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"
#include <filesystem>
#include <fstream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <tiny_gltf.h>
#include <vector>

namespace hiddenpiggy {

struct gltfVertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv0;
  glm::vec2 uv1;

  static vk::VertexInputBindingDescription getBindingDescription() {
    vk::VertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(gltfVertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;
    return bindingDescription;
  }

  static std::array<vk::VertexInputAttributeDescription, 4>
  getAttributeDescriptions() {
    std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions{};

    // Position attribute
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[0].offset = offsetof(gltfVertex, position);

    // Normal attribute
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[1].offset = offsetof(gltfVertex, normal);

    // Texture coordinate attribute
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
    attributeDescriptions[2].offset = offsetof(gltfVertex, uv0);

    // Texture coordinate attribute
    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 2;
    attributeDescriptions[3].format = vk::Format::eR32G32Sfloat;
    attributeDescriptions[3].offset = offsetof(gltfVertex, uv1);
    return attributeDescriptions;
  }
};

struct Primitive {
  uint32_t indexCount = -1;
  uint32_t firstIndex = -1;
  uint32_t vertexCount = 0;
  int32_t vertexOffset = 0;
  uint32_t firstVertex = 0;
  uint32_t materialIndex = 0;
};

struct gltfMesh {
  std::vector<gltfVertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<Primitive> primitives;
};

class glTFModel {
public:
  void loadModel(const char *filePath) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);
    if (!err.empty()) {
      throw std::runtime_error(err);
    }
    if (!warn.empty()) {
      std::cerr << "Warning: " << warn << std::endl;
    }
    if (!ret) {
      throw std::runtime_error("model load failed");
    }

    // handle byte buffer objects
    struct ByteBuffer {
      uint8_t *pData = nullptr;
      uint32_t byteLength;
    };
    std::vector<ByteBuffer> byteBuffers{};

    // iterate over buffers
    for (auto &buffer : model.buffers) {
      std::filesystem::path path{filePath};
      std::ifstream opened_file{path.remove_filename().string() + buffer.uri,
                                std::ios_base::in | std::ios_base::binary};
      if (!opened_file.is_open()) {
        throw std::runtime_error("cannot open the binary file");
      }
      // get the buffer size
      opened_file.seekg(0, std::ios::end);
      std::streamsize fileSize = opened_file.tellg();
      opened_file.seekg(0, std::ios::beg);

      ByteBuffer byteBuffer{};
      byteBuffer.byteLength = fileSize;
      byteBuffer.pData = new uint8_t[fileSize];

      // read file data to the buffer
      if (!opened_file.read(reinterpret_cast<char *>(byteBuffer.pData),
                            fileSize)) {
        throw std::runtime_error("buffer read failed");
      }

      byteBuffers.push_back(byteBuffer);
    }

    for (size_t i = 0; i < model.meshes.size(); ++i) {
      gltfMesh gltfMesh{};
      const tinygltf::Mesh &mesh = model.meshes[i];
      for (size_t j = 0; j < mesh.primitives.size(); j++) {
        Primitive gltfPrimitive{};
        const tinygltf::Primitive &primitive = mesh.primitives[j];
        // handle primitive data
        std::vector<glm::vec3> positions{};
        std::vector<glm::vec3> normals{};
        std::vector<glm::vec2> uv0{};
        std::vector<glm::vec2> uv1{};
        for (auto it = primitive.attributes.begin();
             it != primitive.attributes.end(); it++) {
          const std::string &attributeName = it->first;
          int accessorIndex = it->second;
          const tinygltf::Accessor &accessor = model.accessors[accessorIndex];

          if (attributeName == "POSITION") {

            if (accessor.componentType != 5126) {
              throw std::runtime_error("Component type is not float!");
            }

            auto bufferView = model.bufferViews[accessor.bufferView];
            auto pointer = byteBuffers[bufferView.buffer].pData +
                           bufferView.byteOffset + accessor.byteOffset;
            size_t count = accessor.count;

            while (count--) {
              glm::vec3 data = *(reinterpret_cast<glm::vec3 *>(pointer));
              pointer += bufferView.byteStride > 0 ? bufferView.byteStride
                                                   : sizeof(glm::vec3);
              positions.push_back(data);
            }
          }

          if (attributeName == "NORMAL") {

            if (accessor.componentType != 5126) {
              throw std::runtime_error("Component type is not float!");
            }

            auto bufferView = model.bufferViews[accessor.bufferView];
            auto pointer = byteBuffers[bufferView.buffer].pData +
                           bufferView.byteOffset + accessor.byteOffset;
            size_t count = accessor.count;

            while (count--) {
              glm::vec3 data = *(reinterpret_cast<glm::vec3 *>(pointer));
              pointer += bufferView.byteStride > 0 ? bufferView.byteStride
                                                   : sizeof(glm::vec3);
              normals.push_back(data);
            }
          }

          if (attributeName == "TEXCOORD_0") {
            if (accessor.componentType != 5126) {
              throw std::runtime_error("Component type is not float!");
            }

            auto bufferView = model.bufferViews[accessor.bufferView];
            auto pointer = byteBuffers[bufferView.buffer].pData +
                           bufferView.byteOffset + accessor.byteOffset;
            size_t count = accessor.count;

            while (count--) {
              glm::vec2 data = *(reinterpret_cast<glm::vec2 *>(pointer));
              pointer += bufferView.byteStride > 0 ? bufferView.byteStride
                                                   : sizeof(glm::vec2);
              uv0.push_back(data);
            }
          }
        }

        gltfPrimitive.vertexOffset =
            gltfMesh.vertices.size() * sizeof(glm::vec3);
        gltfPrimitive.firstVertex = gltfMesh.vertices.size();
        for (size_t i = 0; i < positions.size(); ++i) {
          gltfVertex vertex{positions[i],
                            normals.size() > 0 ? normals[i] : glm::vec3(0.0f),
                            uv0.size() > 0 ? uv0[i] : glm::vec3(0.0f)};
          gltfMesh.vertices.push_back(vertex);
        }

        if (primitive.indices >= 0) {
          this->hasIndices = true;
          const tinygltf::Accessor &accessor =
              model.accessors[primitive.indices];
          auto bufferView = model.bufferViews[accessor.bufferView];
          auto pointer = byteBuffers[bufferView.buffer].pData +
                         bufferView.byteOffset + accessor.byteOffset;
          size_t count = accessor.count;

          gltfPrimitive.indexCount = count;
          gltfPrimitive.firstIndex = gltfMesh.indices.size();

          while (count--) {
            // ushort
            if (accessor.componentType == 5123) {
              auto data = *(reinterpret_cast<uint16_t *>(pointer));
              pointer += bufferView.byteStride > 0 ? bufferView.byteStride
                                                   : sizeof(uint16_t);
              gltfMesh.indices.push_back(data);
            }

            // uint
            if (accessor.componentType == 5125) {
              auto data = *(reinterpret_cast<uint32_t *>(pointer));
              pointer += bufferView.byteStride > 0 ? bufferView.byteStride
                                                   : sizeof(uint32_t);
              gltfMesh.indices.push_back(data);
            }
          }
        }

        gltfMesh.primitives.push_back(gltfPrimitive);
      }
      this->meshes.push_back(gltfMesh);
    }

    // clean buffers
    for (auto &byteBuffer : byteBuffers) {
      delete[] byteBuffer.pData;
    }
  }

  void AllocateBuffersAndUpload(BufferPool *bufferPool,
                                ResourceUploadHeap *resourceUploadHeap,
                                uint32_t queueFamilyIndex) {
    m_bufferPool = bufferPool;
    // vertex buffer creation and upload data
    {
      // Calc total size of vertex buffer
      vk::DeviceSize vertexBufferSize = 0;
      for (auto &mesh : meshes) {
        vertexBufferSize += sizeof(gltfVertex) * mesh.vertices.size();
      }
      // Allocate Vertex Buffers
      vk::BufferCreateInfo vertexBufferCreateInfo{
          {},
          vertexBufferSize,
          vk::BufferUsageFlagBits::eVertexBuffer |
              vk::BufferUsageFlagBits::eTransferDst,
          vk::SharingMode::eExclusive,
          1,
          &queueFamilyIndex,
          nullptr};

      // memory allocinfo
      VmaAllocationCreateInfo allocCreateInfo{};
      allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
      vertexBuffer =
          bufferPool->allocateMemory(vertexBufferCreateInfo, allocCreateInfo);

      uint32_t offset = 0;
      for (auto &mesh : meshes) {
        resourceUploadHeap->uploadBufferData(
            mesh.vertices.data(), mesh.vertices.size() * sizeof(gltfVertex),
            vertexBuffer.buffer, vertexBuffer.allocation, offset);
        offset += mesh.vertices.size() * sizeof(gltfVertex);
      }
    }

    // index buffer creation and upload data
    if (this->hasIndices) {
      vk::DeviceSize indexBufferSize = 0;
      for (auto &mesh : meshes) {
        indexBufferSize += sizeof(uint32_t) * mesh.indices.size();
      }
      // Allocate Vertex Buffers
      vk::BufferCreateInfo indexBufferCreateInfo{
          {},
          indexBufferSize,
          vk::BufferUsageFlagBits::eIndexBuffer |
              vk::BufferUsageFlagBits::eTransferDst,
          vk::SharingMode::eExclusive,
          1,
          &queueFamilyIndex,
          nullptr};

      // memory allocinfo
      VmaAllocationCreateInfo allocCreateInfo{};
      allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
      indexBuffer =
          bufferPool->allocateMemory(indexBufferCreateInfo, allocCreateInfo);

      uint32_t offset = 0;
      for (auto &mesh : meshes) {
        resourceUploadHeap->uploadBufferData(
            mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t),
            indexBuffer.buffer, indexBuffer.allocation, offset);
        offset += mesh.indices.size() * sizeof(gltfVertex);
      }
    }
  }

  void draw(vk::CommandBuffer cmdBuf) {
    vk::Buffer buffers[] = {vertexBuffer.buffer};
    size_t offsets[] = {0};
    cmdBuf.bindVertexBuffers(0, buffers, offsets);
    cmdBuf.bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);
    if (hasIndices) {
      for (auto &mesh : meshes) {
        for (auto &primitive : mesh.primitives) {
            cmdBuf.drawIndexed(primitive.indexCount, 1, primitive.firstIndex,
                             primitive.vertexOffset, 0);
        }
      }
    } else {
      for (auto &mesh : meshes) {
        for (auto &primitive : mesh.primitives) {
          cmdBuf.draw(primitive.vertexCount, 1, primitive.firstVertex, 0);
        }
      }
    }
  }

  void destroy() {
    // clean mesh data
    for (auto &mesh : meshes) {
      mesh.vertices.clear();
      mesh.indices.clear();
      mesh.primitives.clear();
    }

    // destroy buffer
    m_bufferPool->freeBuffer(vertexBuffer);
    m_bufferPool->freeBuffer(indexBuffer);
  }


  glm::mat4 getModelMatrix() {
    glm::mat4 identity = glm::identity<glm::mat4>();
    auto scaleMatrix = glm::scale(identity, glm::vec3(scale));
    glm::mat4 rotationMatrix = glm::toMat4(this->rotation);
    glm::mat4 translationMatrix = glm::translate(identity, position);
    return translationMatrix * rotationMatrix * scaleMatrix;
  }

  void setScale(float scale) {
    this->scale = scale;
  }

  void setRotation(float yaw, float pitch, float roll) {
    this->rotation = glm::quat(glm::vec3(yaw, pitch, roll));
  }

  void rotate(float delta, glm::vec3 axis) {
    axis = glm::normalize(axis);
    this->rotation *= glm::quat(cos(delta/2), axis.x * sin(delta/2), axis.y * sin(delta/2), axis.z * sin(delta/2));
  }


private:
  std::vector<gltfMesh> meshes{};
  std::vector<tinygltf::Material> materials{};
  std::vector<tinygltf::Texture> textures{};
  bool hasIndices = false;
  BufferWrapper vertexBuffer;
  BufferWrapper indexBuffer;
  BufferPool *m_bufferPool;

  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::quat rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));
  float scale = 1.0f;
};
}; // namespace hiddenpiggy
#endif