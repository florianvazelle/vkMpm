#ifndef GRAPHICRENDERPASS_HPP
#define GRAPHICRENDERPASS_HPP

#include <poike/poike.hpp>

using namespace poike;

namespace vkm {
  class GraphicRenderPass : public RenderPass {
  public:
    GraphicRenderPass(const Device& device, const SwapChain& swapChain);

  private:
    void createRenderPass() final;
    void createFrameBuffers() final;
  };
}  // namespace vkm

#endif  // GRAPHICRENDERPASS_HPP