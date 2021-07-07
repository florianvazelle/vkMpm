// clang-format off
#include <Compute/ComputeDescriptorSets.hpp>
#include <stddef.h>                           // for size_t
#include <struct/ComputeParticle.hpp>  // for ComputeParticle
#include <struct/Particle.hpp>         // for Particle
#include <poike/poike.hpp>
// clang-format on

using namespace vkm;
using namespace poike;

void ComputeDescriptorSets::createDescriptorSets() {
  {
    /* Allocate */
    const size_t size = 1;

    const std::vector<VkDescriptorSetLayout> layouts(size, m_descriptorSetLayout.handle());
    const VkDescriptorSetAllocateInfo allocInfo
        = misc::descriptorSetAllocateInfo(m_descriptorPool.handle(), layouts.data(), static_cast<uint32_t>(size));

    m_descriptorSets.resize(size);
    if (vkAllocateDescriptorSets(m_device.logical(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
      throw std::runtime_error("echec de l'allocation d'un set de descripteurs!");
    }
  }

  std::vector<VkWriteDescriptorSet> writeDescriptorSets;

  const VkDescriptorBufferInfo psInfo   = m_buffers[0]->descriptor();
  const VkDescriptorBufferInfo gridInfo = m_buffers[1]->descriptor();
  const VkDescriptorBufferInfo fsInfo   = m_buffers[2]->descriptor();

  // On paramètre les descripteurs (on se rappelle que l'on en a mit un par frame)
  const IUniformBuffers* ubo = m_uniformBuffers[0];
  for (size_t i = 0; i < m_descriptorSets.size(); i++) {
    const VkDescriptorBufferInfo bufferInfo = ubo->descriptor(i);

    writeDescriptorSets = {
        misc::writeDescriptorSet(m_descriptorSets.at(i), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &psInfo),
        misc::writeDescriptorSet(m_descriptorSets.at(i), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &gridInfo),
        misc::writeDescriptorSet(m_descriptorSets.at(i), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, &fsInfo),
        misc::writeDescriptorSet(m_descriptorSets.at(i), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &bufferInfo),
    };

    vkUpdateDescriptorSets(m_device.logical(), static_cast<uint32_t>(writeDescriptorSets.size()),
                           writeDescriptorSets.data(), 0, nullptr);
  }
}
