// clang-format off
#include <Graphic/GraphicDescriptorSets.hpp>
#include <stddef.h>                       // for size_t
#include <Struct/MVP.hpp>          // for MVP
#include <poike/poike.hpp>
// clang-format on

using namespace vkl;
using namespace poike;

void GraphicDescriptorSets::createDescriptorSets() {
  allocateDescriptorSets();

  std::vector<VkWriteDescriptorSet> writeDescriptorSets;

  // On paramètre les descripteurs (on se rappelle que l'on en a mit un par frame)
  const IUniformBuffers* ubo = m_uniformBuffers[0];
  for (size_t i = 0; i < m_descriptorSets.size(); i++) {
    const VkDescriptorBufferInfo& bufferInfo = ubo->descriptor(i);

    writeDescriptorSets = {
        // Binding 2 :
        misc::writeDescriptorSet(m_descriptorSets.at(i), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo),
    };

    vkUpdateDescriptorSets(m_device.logical(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
  }
}
