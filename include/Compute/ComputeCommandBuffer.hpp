#ifndef COMPUTECOMMANDBUFFER_HPP
#define COMPUTECOMMANDBUFFER_HPP

#include <poike/poike.hpp>
#include <vector>

using namespace poike;

namespace vkl {

  class ComputeCommandBuffer : public NoCopy {
  public:
    ComputeCommandBuffer(const Device& device,
                         const RenderPass& renderPass,
                         const ComputePipeline& computePipeline,
                         const std::vector<const IBuffer*>& storageBuffers,
                         const CommandPool& commandPool,
                         const DescriptorSets& descriptorSets);
    void recreate();

    inline VkCommandBuffer& command() { return m_commandBuffer; }
    inline const VkCommandBuffer& command() const { return m_commandBuffer; }

  protected:
    VkCommandBuffer m_commandBuffer;

    const Device& m_device;
    const RenderPass& m_renderPass;
    const ComputePipeline& m_computePipeline;
    const std::vector<const IBuffer*>& m_storageBuffers;
    const CommandPool& m_commandPool;
    const DescriptorSets& m_descriptorSets;

    void createCommandBuffers();
    void destroyCommandBuffers();

    // allocate one command buffer
    VkCommandBuffer allocCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false) const;
  };

}  // namespace vkl

#endif  // COMPUTECOMMANDBUFFER_HPP
