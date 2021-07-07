#ifndef GRAPHICGRAPHICSPIPELINE_HPP
#define GRAPHICGRAPHICSPIPELINE_HPP

#include <poike/poike.hpp>

using namespace poike;

namespace vkm {

  class GraphicGraphicsPipeline : public GraphicsPipeline {
  public:
    GraphicGraphicsPipeline(const Device& device,
                            const SwapChain& swapChain,
                            const RenderPass& renderPass,
                            const DescriptorSetLayout& descriptorSetLayout);
    ~GraphicGraphicsPipeline();

  private:
    void createPipeline() final;
  };
}  // namespace vkm

#endif  // GRAPHICGRAPHICSPIPELINE_HPP