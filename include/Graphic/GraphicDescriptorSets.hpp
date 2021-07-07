#ifndef GRAPHICDESCRIPTORSETS_HPP
#define GRAPHICDESCRIPTORSETS_HPP

#include <vector>                     // for vector
#include <poike/poike.hpp>

using namespace poike;

namespace vkl {

  class GraphicDescriptorSets : public DescriptorSets {
  public:
    GraphicDescriptorSets(const Device& device,
                          const SwapChain& swapChain,
                          const DescriptorSetLayout& descriptorSetLayout,
                          const DescriptorPool& descriptorPool,
                          const std::vector<const IBuffer*>& buffers,
                          const std::vector<const IUniformBuffers*>& uniformBuffers)
        : DescriptorSets(device,
                         swapChain,
                         descriptorSetLayout,
                         descriptorPool,
                         buffers,
                         uniformBuffers) {
      createDescriptorSets();
    }

  private:
    void createDescriptorSets() final;
  };
}  // namespace vkl

#endif  // GRAPHICDESCRIPTORSETS_HPP
