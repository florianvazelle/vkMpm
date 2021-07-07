// clang-format off
#include <Compute/ComputePipeline.hpp>
#include <ClearGrid_comp.h>    
#include <P2G_comp.h>        
#include <UpdateGrid_comp.h>   
#include <G2P_comp.h>   
#include <poike/poike.hpp>
#include <glm/glm.hpp>
#include <stdexcept>                         // for runtime_error
#include <map>
#include <iostream>
// clang-format on

using namespace vkl;
using namespace poike;

ComputePipeline::ComputePipeline(const Device& device,
                                 const SwapChain& swapChain,
                                 const RenderPass& renderPass,
                                 const DescriptorSetLayout& descriptorSetLayout)
    : GraphicsPipeline(device, swapChain, renderPass, descriptorSetLayout), m_pipelines(4) {
  createPipeline();
}

ComputePipeline::~ComputePipeline() { destroyComputePipeline(); }

void ComputePipeline::recreate() {
  destroyComputePipeline();
  createPipeline();
}

void ComputePipeline::destroyComputePipeline() {
  for (size_t i = 0; i < m_pipelines.size(); i++) {
    vkDestroyPipeline(m_device.logical(), m_pipelines[i], nullptr);
  }

  destroyPipeline();
}

void ComputePipeline::createPipeline() {
  {
    const VkDescriptorSetLayout layouts[]               = {m_descriptorSetLayout.handle()};
    const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts    = layouts,
    };

    if (vkCreatePipelineLayout(m_device.logical(), &pipelineLayoutInfo, nullptr, &m_layout) != VK_SUCCESS) {
      throw std::runtime_error("Pipeline Layout creation failed");
    }
  }

  VkComputePipelineCreateInfo computePipelineCreateInfo = {
      .sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .flags  = 0,
      .layout = m_layout,
  };

  {  // 1st pass
    VkShaderModule compShaderModule = createShaderModule(CLEARGRID_COMP);
    computePipelineCreateInfo.stage
        = misc::pipelineShaderStageCreateInfo(compShaderModule, VK_SHADER_STAGE_COMPUTE_BIT);

    if (vkCreateComputePipelines(m_device.logical(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
                                 &m_pipelines[0])
        != VK_SUCCESS) {
      throw std::runtime_error("Compute Pipeline Calculate creation failed");
    }

    deleteShaderModule({computePipelineCreateInfo.stage});
  }

  {  // 2nd pass
    VkShaderModule compShaderModule = createShaderModule(P2G_COMP);
    computePipelineCreateInfo.stage
        = misc::pipelineShaderStageCreateInfo(compShaderModule, VK_SHADER_STAGE_COMPUTE_BIT);

    if (vkCreateComputePipelines(m_device.logical(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
                                 &m_pipelines[1])
        != VK_SUCCESS) {
      throw std::runtime_error("Compute Pipeline Integrate creation failed");
    }

    deleteShaderModule({computePipelineCreateInfo.stage});
  }

  {  // 1st pass
    VkShaderModule compShaderModule = createShaderModule(UPDATEGRID_COMP);
    computePipelineCreateInfo.stage
        = misc::pipelineShaderStageCreateInfo(compShaderModule, VK_SHADER_STAGE_COMPUTE_BIT);

    if (vkCreateComputePipelines(m_device.logical(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
                                 &m_pipelines[2])
        != VK_SUCCESS) {
      throw std::runtime_error("Compute Pipeline Calculate creation failed");
    }

    deleteShaderModule({computePipelineCreateInfo.stage});
  }
  {
    // 2nd pass
    VkShaderModule compShaderModule = createShaderModule(G2P_COMP);
    computePipelineCreateInfo.stage
        = misc::pipelineShaderStageCreateInfo(compShaderModule, VK_SHADER_STAGE_COMPUTE_BIT);

    if (vkCreateComputePipelines(m_device.logical(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr,
                                 &m_pipelines[3])
        != VK_SUCCESS) {
      throw std::runtime_error("Compute Pipeline Integrate creation failed");
    }

    deleteShaderModule({computePipelineCreateInfo.stage});
  }
}