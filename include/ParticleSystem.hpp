/**
 * @file ParticleSystem.hpp
 * @brief Define ParticleSystem class
 *
 * This is our Vulkan App, call by the main and run the main loop.
 */

#pragma once

#include <poike/poike.hpp>
#include <struct/ComputeParticle.hpp>             // for ComputeParticle
#include <struct/ParticleMVP.hpp>                 // for ParticleMVP
#include <struct/Particle.hpp>                    // for Particle
#include <cstdlib>                                       // for size_t
#include <functional>                                    // for function
#include <Compute/ComputeCommandBuffer.hpp>     // for ComputeComma...
#include <Compute/MPMStorageBuffer.hpp>
#include <Compute/ComputeDescriptorSets.hpp>    // for ComputeDescr...
#include <Compute/ComputePipeline.hpp>          // for ComputePipeline
#include <Graphic/GraphicCommandBuffers.hpp>    // for GraphicComma...
#include <Graphic/GraphicDescriptorSets.hpp>    // for GraphicDescr...
#include <Graphic/GraphicGraphicsPipeline.hpp>  // for GraphicGraph...
#include <Graphic/GraphicRenderPass.hpp>              // for GraphicRenderPass
#include <string>                                        // for string
#include <vector>                                        // for vector

using namespace poike;

namespace vkm {
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
    GraphicRenderPass rpGraphic;

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

}  // namespace vkm
