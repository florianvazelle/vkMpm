#ifndef GRAPHICCOMMANDBUFFERS_HPP
#define GRAPHICCOMMANDBUFFERS_HPP

#include <poike/poike.hpp>
#include <vector>  // for vector

using namespace poike;

namespace vkm {

  class GraphicCommandBuffers : public CommandBuffers {
  public:
    GraphicCommandBuffers(const Device& device,
                          const RenderPass& renderPass,
                          const SwapChain& swapChain,
                          const GraphicsPipeline& graphicsPipeline,
                          const CommandPool& commandPool,
                          const DescriptorSets& descriptorSets,
                          const std::vector<const IBuffer*>& buffers)
        : CommandBuffers(device, renderPass, swapChain, graphicsPipeline, commandPool, descriptorSets, buffers) {
      createCommandBuffers();
    }

  private:
    void createCommandBuffers() final;
  };

}  // namespace vkm

#endif  // GRAPHICCOMMANDBUFFERS_HPP
