// clang-format off
#include <particle/Graphic/GraphicCommandBuffers.hpp>
#include <stddef.h>                         // for size_t
#include <stdint.h>                         // for uint32_t
#include <common/VulkanHeader.hpp>             // for VkCommandBuffer, VkComman...
#include <stdexcept>                        // for runtime_error
#include <common/CommandPool.hpp>           // for CommandPool, vkl
#include <common/DescriptorSets.hpp>        // for DescriptorSets
#include <common/Device.hpp>                // for Device
#include <common/GraphicsPipeline.hpp>      // for GraphicsPipeline
#include <common/RenderPass.hpp>            // for RenderPass
#include <common/SwapChain.hpp>             // for SwapChain
#include <common/buffer/StorageBuffer.hpp>  // for StorageBuffer
#include <particle/Compute/MPMStorageBuffer.hpp>
// clang-format on

using namespace vkl;

void GraphicCommandBuffers::createCommandBuffers() {
  m_commandBuffers.resize(m_renderPass.size());

  const VkCommandBufferAllocateInfo allocInfo = {
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = m_commandPool.handle(),
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size()),
  };

  if (vkAllocateCommandBuffers(m_device.logical(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }

  const VkCommandBufferBeginInfo cmdBufInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
  };

  VkClearValue clearValues[2];
  clearValues[0].color        = {{0.0f, 0.0f, 1.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo renderPassBeginInfo = {
          .sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
          .renderPass        = m_renderPass.handle(),
          .renderArea = {
              .offset = {0, 0},
              .extent = m_swapChain.extent(),
          },
          .clearValueCount   = 2,
          .pClearValues      = clearValues,
      };

  for (size_t i = 0; i < m_commandBuffers.size(); ++i) {
    // Set target frame buffer
    renderPassBeginInfo.framebuffer = m_renderPass.frameBuffer(i);

    if (vkBeginCommandBuffer(m_commandBuffers.at(i), &cmdBufInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    const StorageBuffer* storageBuffer = dynamic_cast<const StorageBuffer*>(m_buffers[0]);

    const std::optional<uint32_t>& graphicsFamily = m_device.queueFamilyIndices().graphicsFamily;
    const std::optional<uint32_t>& computeFamily  = m_device.queueFamilyIndices().computeFamily;

    // Acquire barrier
    if (graphicsFamily.value() != computeFamily.value()) {
      const VkBufferMemoryBarrier buffer_barrier = {
          .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
          .srcAccessMask       = 0,
          .dstAccessMask       = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
          .srcQueueFamilyIndex = computeFamily.value(),
          .dstQueueFamilyIndex = graphicsFamily.value(),
          .buffer              = storageBuffer->buffer(),
          .offset              = 0,
          .size                = storageBuffer->size(),
      };

      vkCmdPipelineBarrier(m_commandBuffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0, nullptr);
    }

    // Draw the particle system using the update vertex buffer
    vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline.pipeline());
    vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline.layout(), 0, 1,
                            &m_descriptorSets.descriptor(i), 0, nullptr);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, &(storageBuffer->buffer()), offsets);
    vkCmdDraw(m_commandBuffers[i], NUM_PARTICLE, 1, 0, 0);

    vkCmdEndRenderPass(m_commandBuffers[i]);

    // Release barrier
    if (graphicsFamily.value() != computeFamily.value()) {
      const VkBufferMemoryBarrier buffer_barrier = {
          .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
          .srcAccessMask       = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
          .dstAccessMask       = 0,
          .srcQueueFamilyIndex = graphicsFamily.value(),
          .dstQueueFamilyIndex = computeFamily.value(),
          .buffer              = storageBuffer->buffer(),
          .offset              = 0,
          .size                = storageBuffer->size(),
      };

      vkCmdPipelineBarrier(m_commandBuffers[i], VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &buffer_barrier, 0, nullptr);
    }

    if (vkEndCommandBuffer(m_commandBuffers.at(i)) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}
