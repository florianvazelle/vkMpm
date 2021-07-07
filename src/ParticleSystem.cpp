// clang-format off
#include <ParticleSystem.hpp>
#include <chrono>                                        // for duration
#include <cstdint>                                       // for uint32_t
#include <cstring>                                       // for memcpy
#include <deque>                                         // for deque
#include <memory>                                        // for allocator_tr...
#include <stdexcept>                                     // for runtime_error
#include <poike/poike.hpp>
#include <struct/ComputeParticle.hpp>             // for ComputeParticle
#include <struct/ParticleMVP.hpp>                 // for ParticleMVP
#include <struct/Particle.hpp>                    // for Particle
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifndef __ANDROID__
#include <imgui.h>                                       // for Text, Begin
#include <imgui_impl_glfw.h>                             // for ImGui_ImplGl...
#include <imgui_impl_vulkan.h>                           // for ImGui_ImplVu...
#endif
#include <Compute/ComputeCommandBuffer.hpp>     // for ComputeComma...
#include <Compute/ComputeDescriptorSets.hpp>    // for ComputeDescr...
#include <Graphic/GraphicDescriptorSets.hpp>    // for GraphicDescr...
#include <Graphic/GraphicGraphicsPipeline.hpp>  // for GraphicGraph...
#include <Graphic/GraphicRenderPass.hpp>              // for GraphicRenderPass
#include <glm/gtc/matrix_transform.hpp>
// clang-format on

using namespace vkl;
using namespace poike;

static bool isPause = true;

void updateGraphicsUniformBuffers(const Device& device,
                                  const SwapChain& swapChain,
                                  std::deque<Buffer<ParticleMVP>>& uniformBuffers,
                                  float time,
                                  uint32_t currentImage) {
  ParticleMVP& ubo = uniformBuffers.at(currentImage).data().at(0);

  ubo.model = glm::mat4(1.0f);

  glm::mat4 rotM = glm::mat4(1.0f);
  // rotM           = glm::rotate(rotM, glm::radians(-26.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  // rotM           = glm::rotate(rotM, glm::radians(75.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  // rotM           = glm::rotate(rotM, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

  ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(-32.0f, -32.0f, -5.0f)) * rotM;

  const float aspect = swapChain.extent().width / (float)swapChain.extent().height;
  ubo.proj           = glm::perspective(60.0f, aspect, 0.1f, 512.0f);
  ubo.proj[1][1] *= -1;

  ubo.screenDim = glm::vec2(swapChain.extent().width, swapChain.extent().height);

  void* data;
  vkMapMemory(device.logical(), uniformBuffers[currentImage].memory(), 0, sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(device.logical(), uniformBuffers[currentImage].memory());
}

void updateComputeUniformBuffers(const Device& device,
                                 const SwapChain& swapChain,
                                 std::deque<Buffer<ComputeParticle>>& uniformBuffers,
                                 float time,
                                 uint32_t currentImage) {
  ComputeParticle& ubo = uniformBuffers.at(currentImage).data().at(0);

  ubo.deltaT         = isPause ? 0.0f : DT;
  ubo.particleCount  = NUM_PARTICLE;
  ubo.elastic_lambda = elastic_lambda;
  ubo.elastic_mu     = elastic_mu;

  void* data;
  vkMapMemory(device.logical(), uniformBuffers[currentImage].memory(), 0, sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(device.logical(), uniformBuffers[currentImage].memory());
}

ParticleSystem::ParticleSystem(
#ifdef __ANDROID__
    android_app* androidApp,
#endif
    const std::string& appName,
    const DebugOption& debugOption)
    : Application(
#ifdef __ANDROID__
        androidApp,
#endif
        appName,
        debugOption),

      commandPool(device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
      // Use a separate command pool (queue family may differ from the one used for graphics)
      commandPoolCompute(device,
                         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                         device.queueFamilyIndices().computeFamily),

      // Descriptor Pool
      ps({
          misc::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChain.numImages()),
      }),
      dpi(misc::descriptorPoolCreateInfo(ps, swapChain.numImages())),
      dp(device, dpi),

      psCompute({
          misc::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
          misc::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3),
      }),
      dpiCompute(misc::descriptorPoolCreateInfo(psCompute, 4)),
      dpCompute(device, dpiCompute),

      // Buffer
      // Graphic
      uniformBuffersGraphic(device, swapChain, &updateGraphicsUniformBuffers),

      // Compute
      storageBuffer(device,
                    commandPool,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
      uniformBuffersCompute(device, swapChain, &updateComputeUniformBuffers),

      // ~ My Vectors
      // Utile car sinon les pointeurs change, donc on copie d'abord par valeur
      // et on passe le vecteur qui sera concervé dans la class Application
      vecUBGraphic({&uniformBuffersGraphic}),
      vecUBCompute({&uniformBuffersCompute}),
      vecSBCompute({&storageBuffer.ps, &storageBuffer.grid, &storageBuffer.fs}),

      /*
       * Basic Graphics
       */

      // 1. Render Pass
      rpGraphic(device, swapChain),

      // 2. Descriptor Set Layout
      dslGraphic(device,
                 misc::descriptorSetLayoutCreateInfo({
                     misc::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
                 })),

      // 3. Graphic Pipeline
      gpGraphic(device, swapChain, rpGraphic, dslGraphic),

      // 5. Descriptor Sets
      dsGraphic(device, swapChain, dslGraphic, dp, {}, vecUBGraphic),

      // Semaphore for compute & graphics sync
      semaphoreGraphic(device),

      /*
       * Compute
       */

      // 2. Descriptor Set Layout
      dslCompute(
          device,
          misc::descriptorSetLayoutCreateInfo({
              // Binding 0 : Particle position storage buffer
              misc::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 0),
              misc::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 1),
              misc::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 2),
              // Binding 1 : Uniform buffer
              misc::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, 3),
          })),

      // 5. Descriptor Sets
      dsCompute(device, swapChain, dslCompute, dpCompute, vecSBCompute, vecUBCompute),

      // 3. Compute Pipeline
      gpCompute(device, swapChain, rpGraphic, dslCompute),

      semaphoreCompute(device),

      cbCompute(device, rpGraphic, gpCompute, vecSBCompute, commandPoolCompute, dsCompute),

      cbGraphic(device, rpGraphic, swapChain, gpGraphic, commandPool, dsGraphic, vecSBCompute)
#ifndef __ANDROID__
      /* ImGui */
      ,
      interface(instance, window, device, swapChain, gpGraphic)
#endif
{
  // Trigger compute semaphore, to start by compute queue
  {
    const VkSubmitInfo submitInfo = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &semaphoreCompute.handle(),
    };

    // maybe graphics queue
    if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit signal semaphore!");
    }
    vkQueueWaitIdle(device.graphicsQueue());
  }
}

void ParticleSystem::run() {
  window.setDrawFrameFunc([this](bool& framebufferResized) {
    drawImGui();
    drawFrame(framebufferResized);
  });

  window.mainLoop();
  vkDeviceWaitIdle(device.logical());
}

void ParticleSystem::drawFrame(bool& framebufferResized) {
  uint32_t imageIndex;
  VkResult result = prepareFrame(false, framebufferResized, imageIndex);
  if (result != VK_SUCCESS) return;

    /* Buffer */
#ifndef __ANDROID__
  interface.recordCommandBuffers(imageIndex);
#endif

  /* Update Uniform Buffers */

  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time       = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  uniformBuffersGraphic.update(time, imageIndex);
  uniformBuffersCompute.update(time, imageIndex);

  /* Submit graphics commands */
  {
    const std::vector<VkCommandBuffer> cmdBuffers = {
        cbGraphic.command(imageIndex),
#ifndef __ANDROID__
        interface.command(imageIndex),
#endif
    };

    const VkPipelineStageFlags waitStageMasks[] = {
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    const VkSemaphore waitSemaphores[]   = {semaphoreCompute.handle(), syncObjects.imageAvailable(currentFrame)};
    const VkSemaphore signalSemaphores[] = {semaphoreGraphic.handle(), syncObjects.renderFinished(currentFrame)};

    const VkSubmitInfo submitInfo = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = 2,
        .pWaitSemaphores      = waitSemaphores,
        .pWaitDstStageMask    = waitStageMasks,
        .commandBufferCount   = static_cast<uint32_t>(cmdBuffers.size()),
        .pCommandBuffers      = cmdBuffers.data(),
        .signalSemaphoreCount = 2,
        .pSignalSemaphores    = signalSemaphores,
    };

    // vkResetFences(device.logical(), 1, &syncObjects.inFlightFence(currentFrame));

    if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }
  }

  submitFrame(false, framebufferResized, imageIndex);

  /* Submit compute commands */
  {
    const std::vector<VkCommandBuffer> cmdBuffers = {
        cbCompute.command(),
    };

    const VkPipelineStageFlags waitStageMasks[] = {VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT};
    const VkSemaphore waitSemaphores[]          = {semaphoreGraphic.handle()};
    const VkSemaphore signalSemaphores[]        = {semaphoreCompute.handle()};

    const VkSubmitInfo computeSubmitInfo = {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = waitSemaphores,
        .pWaitDstStageMask    = waitStageMasks,
        .commandBufferCount   = static_cast<uint32_t>(cmdBuffers.size()),
        .pCommandBuffers      = cmdBuffers.data(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = signalSemaphores,
    };

    if (vkQueueSubmit(device.computeQueue(), 1, &computeSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
    }
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void ParticleSystem::drawImGui() {
#ifndef __ANDROID__
  // Start the Dear ImGui frame
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time       = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  // Display ImGui components
  ImGui::Begin("Config");

  {
    static int frame = 0;
    ImGui::Text("frame: %d", ++frame);
    ImGui::Text("time: %.2f", time);
    ImGui::Text("fps: %.2f", ImGui::GetIO().Framerate);

    ImGui::Separator();
    if (ImGui::Button(isPause ? "Play" : "Pause")) {
      isPause = !isPause;
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart")) {
      storageBuffer.recreate();
    }

    ImGui::Separator();
    ImGui::Text("MPM Settings");
    if (ImGui::Button("Solid")) {
      elastic_lambda = 10.0f;
      elastic_mu     = 20.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Liquid")) {
      elastic_lambda = 100.0f;
      elastic_mu     = 0.1f;
    }

    ImGui::SliderFloat("lambda", &(elastic_lambda), 10.0f, 100.0f);
    ImGui::SliderFloat("mu", &(elastic_mu), 0.1f, 20.0f);
  }

  ImGui::End();
  ImGui::Render();
#endif
}

// for resize window
void ParticleSystem::recreateSwapChain(bool& framebufferResized) {
  glm::ivec2 size;
  window.framebufferSize(size);
  while (size[0] == 0 || size[1] == 0) {
    window.framebufferSize(size);
#ifndef __ANDROID__
    glfwWaitEvents();
#endif
  }

  vkDeviceWaitIdle(device.logical());

  swapChain.recreate();

  // Recreated because the number of buffer is based on number of image in swapchain
  uniformBuffersGraphic.recreate();
  uniformBuffersCompute.recreate();

  /**
   * Graphic
   */
  rpGraphic.recreate();
  gpGraphic.recreate();
  dp.recreate();
  dsGraphic.recreate();
  cbGraphic.recreate();

  /**
   * Compute
   */
  gpCompute.recreate();
  dpCompute.recreate();
  dsCompute.recreate();
  cbCompute.recreate();

#ifndef __ANDROID__
  interface.recreate();
#endif

  swapChain.cleanupOld();
}

#ifdef __ANDROID__
void ParticleSystem::togglePause() const { isPause = !isPause; }
#endif
