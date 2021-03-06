#ifndef COMPUTEDESCRIPTORSETS_HPP
#define COMPUTEDESCRIPTORSETS_HPP

#include <poike/poike.hpp>
#include <vector>  

using namespace poike;

namespace vkm {

  class ComputeDescriptorSets : public DescriptorSets {
  public:
    ComputeDescriptorSets(const Device& device,
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
}  // namespace vkm

#endif  // COMPUTEDESCRIPTORSETS_HPP
