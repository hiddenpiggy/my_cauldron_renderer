#ifndef VK_RENDERPASS_HPP
#define VK_RENDERPASS_HPP
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_structs.hpp"
namespace hiddenpiggy {
class VKRenderPass {
public:
  virtual ~VKRenderPass() {};
  virtual void OnCreate() {};
  virtual void OnDestroy() {};
  virtual void OnExecuteSubpass(uint32_t subpassIndex) {};

protected:
  std::vector<vk::AttachmentDescription>
      m_attachments;                               // attachments description
  std::vector<vk::SubpassDescription> m_subpasses; // subpass description
  std::vector<vk::SubpassDependency>
      m_dependencies; // subpass dependency description
};
} // namespace hiddenpiggy

#endif
