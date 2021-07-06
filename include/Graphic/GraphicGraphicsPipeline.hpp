#ifndef GRAPHICGRAPHICSPIPELINE_HPP
#define GRAPHICGRAPHICSPIPELINE_HPP

// clang-format off
#include <common/GraphicsPipeline.hpp>  // for GraphicsPipeline
namespace vkl { class DescriptorSetLayout; }
namespace vkl { class Device; }
namespace vkl { class RenderPass; }
namespace vkl { class SwapChain; }
// clang-format on

namespace vkl {

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
}  // namespace vkl

#endif  // GRAPHICGRAPHICSPIPELINE_HPP