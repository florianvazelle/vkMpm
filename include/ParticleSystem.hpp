/**
 * @file ParticleSystem.hpp
 * @brief Define ParticleSystem class
 *
 * This is our Vulkan App, call by the main and run the main loop.
 */

#pragma once

// clang-format off
#include <common/VulkanHeader.hpp>                          // for VkDescriptor...
#include <common/Application.hpp>                        // for Application
#include <common/CommandPool.hpp>                        // for CommandPool
#include <common/DescriptorPool.hpp>                     // for DescriptorPool
#ifndef __ANDROID__
#include <common/ImGui/ImGuiApp.hpp>                     // for ImGuiApp
#endif
#include <common/DescriptorSetLayout.hpp>                // for DescriptorSe...
#include <common/Semaphore.hpp>                          // for Semaphore
#include <common/buffer/Buffer.hpp>                      // for Buffer
#include <common/buffer/StorageBuffer.hpp>               // for StorageBuffer
#include <common/buffer/UniformBuffers.hpp>              // for UniformBuffers
#include <common/struct/ComputeParticle.hpp>             // for ComputeParticle
#include <common/struct/ParticleMVP.hpp>                 // for ParticleMVP
#include <common/struct/Particle.hpp>                    // for Particle
#include <cstdlib>                                       // for size_t
#include <functional>                                    // for function
#include <particle/Compute/ComputeCommandBuffer.hpp>     // for ComputeComma...
#include <particle/Compute/MPMStorageBuffer.hpp>
#include <particle/Compute/ComputeDescriptorSets.hpp>    // for ComputeDescr...
#include <particle/Compute/ComputePipeline.hpp>          // for ComputePipeline
#include <particle/Graphic/GraphicCommandBuffers.hpp>    // for GraphicComma...
#include <particle/Graphic/GraphicDescriptorSets.hpp>    // for GraphicDescr...
#include <particle/Graphic/GraphicGraphicsPipeline.hpp>  // for GraphicGraph...
#include <shadow/Basic/BasicRenderPass.hpp>              // for BasicRenderPass
#include <string>                                        // for string
#include <vector>                                        // for vector
// clang-format on

namespace vkl {
  class ParticleSystem : public Application {
  public:
    ParticleSystem(
#ifdef __ANDROID__
        android_app* androidApp,
#endif
        const std::string& appName,
        const DebugOption& debugOption);

    void run();

#ifdef __ANDROID__
    void togglePause() const;
#endif

  private:
    CommandPool commandPool, commandPoolCompute;

    // Descriptor Pool
    const std::vector<VkDescriptorPoolSize> ps;
    VkDescriptorPoolCreateInfo dpi;
    DescriptorPool dp;

    const std::vector<VkDescriptorPoolSize> psCompute;
    VkDescriptorPoolCreateInfo dpiCompute;
    DescriptorPool dpCompute;

    // Buffers
    UniformBuffers<ParticleMVP> uniformBuffersGraphic;
    MPMStorageBuffer storageBuffer;
    UniformBuffers<ComputeParticle> uniformBuffersCompute;

    // Vector Buffer
    std::vector<const IUniformBuffers*> vecUBGraphic;
    std::vector<const IUniformBuffers*> vecUBCompute;
    std::vector<const IBuffer*> vecSBCompute;

    // Graphic
    BasicRenderPass rpGraphic;

    DescriptorSetLayout dslGraphic;
    GraphicGraphicsPipeline gpGraphic;
    GraphicDescriptorSets dsGraphic;
    Semaphore semaphoreGraphic;

    // Compute

    DescriptorSetLayout dslCompute;
    ComputeDescriptorSets dsCompute;
    ComputePipeline gpCompute;
    Semaphore semaphoreCompute;

    ComputeCommandBuffer cbCompute;
    GraphicCommandBuffers cbGraphic;

#ifndef __ANDROID__
    ImGuiApp interface;
#endif

    void drawFrame(bool& framebufferResized);
    void drawImGui();

    void recreateSwapChain(bool& framebufferResized) final;
  };

}  // namespace vkl
