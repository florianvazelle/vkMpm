
#ifndef COMPUTEPIPELINE_HPP
#define COMPUTEPIPELINE_HPP

// clang-format off
#include <common/VulkanHeader.hpp>         // for VkPipeline, VkPipeline_T
#include <common/GraphicsPipeline.hpp>  // for GraphicsPipeline
namespace vkl { class DescriptorSetLayout; }
namespace vkl { class Device; }
namespace vkl { class RenderPass; }
namespace vkl { class SwapChain; }
// clang-format on

namespace vkl {

  class ComputePipeline : public GraphicsPipeline {
  public:
    ComputePipeline(const Device& device,
                    const SwapChain& swapChain,
                    const RenderPass& renderPass,
                    const DescriptorSetLayout& descriptorSetLayout);
    ~ComputePipeline();

    void recreate() final;

    inline const VkPipeline& pipeline(int i) const { return m_pipelines[i]; }

  private:
    std::vector<VkPipeline> m_pipelines;

    void createPipeline() final;
    void destroyComputePipeline();
  };
}  // namespace vkl

#endif  // COMPUTEPIPELINE_HPP