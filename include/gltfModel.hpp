#ifndef _GLTF_MODEL_HPP
#define _GLTF_MODEL_HPP

#include "VkContext.hpp"
#include "vulkan/vulkan.hpp"
#include <vk_mem_alloc.h>

// glm headers
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// tinygltf
#include "tiny_gltf.h"

// here is my bufferpool class
#include "ResourceUploadHeap.hpp"
#include "VkBufferPool.hpp"
#include "VkCommandBuffers.hpp"


// Changing this value here also requires changing it in the vertex shader
#define MAX_NUM_JOINTS 128u

namespace hiddenpiggy {
// former declaration
struct Node;

struct BoundingBox {
  glm::vec3 min;
  glm::vec3 max;
  bool valid = false;
  BoundingBox();
  BoundingBox(glm::vec3 min, glm::vec3 max);
  BoundingBox getAABB(glm::mat4 m);
};

struct TextureSampler {
  vk::Filter magFilter;
  vk::Filter minFilter;
  vk::SamplerAddressMode addressModeU;
  vk::SamplerAddressMode addressModeV;
  vk::SamplerAddressMode addressModeW;
};

struct Texture {
  VkContext *context;
  vk::Image image;
  VmaAllocator allocator;
  VmaAllocation allocation;
  VmaAllocationInfo allocationInfo;
  vk::ImageLayout imageLayout;
  vk::ImageView view;
  uint32_t width, height;
  uint32_t mipLevels;
  uint32_t layerCount;
  vk::DescriptorImageInfo descriptor;
  vk::Sampler sampler;
  BufferPool *bufferPool;
  VkCommandBuffers *commandBuffers;

  void updateDescriptor();
  void setContext(VkContext *pContext);
  void setAllocator(VmaAllocator allocator);
  void setBufferPool(BufferPool *pBufferPool);
  void setCommandBuffers(VkCommandBuffers *pCommandBuffers);
  void destroy();
  void fromglTFImage(tinygltf::Image &gltfImage, TextureSampler textureSampler,
                     vk::Queue copyQueue,
                     ResourceUploadHeap *resourceUploadHeap);
};

struct Material {
  enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
  AlphaMode alphaMode = ALPHAMODE_OPAQUE;
  float alphaCutoff = 1.0f;
  float metallicFactor = 1.0f;
  float roughnessFactor = 1.0f;
  glm::vec4 baseColorFactor = glm::vec4(1.0f);
  glm::vec4 emissiveFactor = glm::vec4(1.0f);

  Texture *baseColorTexture;
  Texture *metallicRoughnessTexture;
  Texture *normalTexture;
  Texture *occlusionTexture;
  Texture *emissiveTexture;

  bool doubleSided = false;

  struct TexCoordSets {
    uint8_t baseColor = 0;
    uint8_t metallicRoughness = 0;
    uint8_t specularGlossiness = 0;
    uint8_t normal = 0;
    uint8_t occlusion = 0;
    uint8_t emissive = 0;
  } texCoordSets;

  struct Extension {
    Texture *specularGlossinessTexture;
    Texture *diffuseTexture;
    glm::vec4 diffuseFactor = glm::vec4(1.0f);
    glm::vec3 specularFactor = glm::vec3(0.0f);
  } extension;

  struct PbrWorkflows {
    bool metallicRoughness = true;
    bool specularGlossiness = false;
  } pbrWorkflows;

  vk::DescriptorSet descriptorSet = {};
};

struct Primitive {
  uint32_t firstIndex;
  uint32_t indexCount;
  uint32_t vertexCount;
  Material &material;
  bool hasIndices;
  BoundingBox bb;
  Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount,
            Material &material);
  void setBoundingBox(glm::vec3 min, glm::vec3 max);
};

struct glTFMesh {
  VkContext *context;
  vk::Device device;
  BufferPool *bufferPool;
  VmaAllocation allocation;
  VmaAllocationInfo allocationInfo;
  std::vector<Primitive *> primitives;
  BoundingBox bb;
  BoundingBox aabb;
  struct UniformBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDescriptorBufferInfo descriptor;
    VkDescriptorSet descriptorSet;
    void *mapped;
  } uniformBuffer;
  struct UniformBlock {
    glm::mat4 matrix;
    glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
    float jointcount{0};
  } uniformBlock;
  glTFMesh(VkContext *context, BufferPool *bufferPool, glm::mat4 matrix);
  ~glTFMesh();
  void setBoundingBox(glm::vec3 min, glm::vec3 max);
};

struct Skin {
  std::string name;
  Node *skeletonRoot = nullptr;
  std::vector<glm::mat4> inverseBindMatrices;
  std::vector<Node *> joints;
};

struct Node {
  Node *parent;
  uint32_t index;
  std::vector<Node *> children;
  glm::mat4 matrix;
  std::string name;
  glTFMesh *mesh;
  Skin *skin;
  int32_t skinIndex = -1;
  glm::vec3 translation{};
  glm::vec3 scale{1.0f};
  glm::quat rotation{};
  BoundingBox bvh;
  BoundingBox aabb;
  glm::mat4 localMatrix();
  glm::mat4 getMatrix();
  void update();
  ~Node();
};

struct AnimationChannel {
  enum PathType { TRANSLATION, ROTATION, SCALE };
  PathType path;
  Node *node;
  uint32_t samplerIndex;
};

struct AnimationSampler {
  enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
  InterpolationType interpolation;
  std::vector<float> inputs;
  std::vector<glm::vec4> outputsVec4;
};

struct Animation {
  std::string name;
  std::vector<AnimationSampler> samplers;
  std::vector<AnimationChannel> channels;
  float start = std::numeric_limits<float>::max();
  float end = std::numeric_limits<float>::min();
};

struct glTFModel {
  VkContext *context;
  vk::Device device;
  BufferPool *bufferPool;
  VkCommandBuffers *commandBuffers;
  ResourceUploadHeap *resourceUploadHeap;
  struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv0;
    glm::vec2 uv1;
    glm::vec4 joint0;
    glm::vec4 weight0;
    glm::vec4 color;
  };

  struct Vertices {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation bufferAllocation;
    VmaAllocationInfo bufferAllocationInfo;
    VkDeviceMemory memory;
  } vertices;
  struct Indices {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation bufferAllocation;
    VmaAllocationInfo bufferAllocationInfo;
    VkDeviceMemory memory;
  } indices;

  glm::mat4 aabb;

  std::vector<Node *> nodes;
  std::vector<Node *> linearNodes;

  std::vector<Skin *> skins;

  std::vector<Texture> textures;
  std::vector<TextureSampler> textureSamplers;
  std::vector<Material> materials;
  std::vector<Animation> animations;
  std::vector<std::string> extensions;

  struct Dimensions {
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);
  } dimensions;

  struct LoaderInfo {
    uint32_t *indexBuffer;
    Vertex *vertexBuffer;
    size_t indexPos = 0;
    size_t vertexPos = 0;
  };

  void destroy(vk::Device device);
  void loadNode(Node *parent, const tinygltf::Node &node, uint32_t nodeIndex,
                const tinygltf::Model &model, LoaderInfo &loaderInfo,
                float globalscale);
  void getNodeProps(const tinygltf::Node &node, const tinygltf::Model &model,
                    size_t &vertexCount, size_t &indexCount);
  void loadSkins(tinygltf::Model &gltfModel);
  void loadTextures(tinygltf::Model &gltfModel, VkContext *context,
                    VkCommandBuffers *commandBuffers, BufferPool *bufferPool,
                    ResourceUploadHeap *resourceUploadHeap,
                    vk::Queue transferQueue);
  vk::SamplerAddressMode getVkWrapMode(int32_t wrapMode);
  vk::Filter getVkFilterMode(int32_t filterMode);
  void loadTextureSamplers(tinygltf::Model &gltfModel);
  void loadMaterials(tinygltf::Model &gltfModel);
  void loadAnimations(tinygltf::Model &gltfModel);
  void loadFromFile(std::string filename, BufferPool *bufferPool,
                    VkContext *context, ResourceUploadHeap *resourceUploadHeap,
                    VkCommandBuffers *commandBuffers, VkQueue transferQueue,
                    float scale = 1.0f);
  void drawNode(Node *node, VkCommandBuffer commandBuffer);
  void draw(vk::CommandBuffer commandBuffer);
  void calculateBoundingBox(Node *node, Node *parent);
  void getSceneDimensions();
  void updateAnimation(uint32_t index, float time);
  Node *findNode(Node *parent, uint32_t index);
  Node *nodeFromIndex(uint32_t index);
};
} // namespace hiddenpiggy

#endif