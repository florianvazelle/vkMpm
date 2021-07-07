// clang-format off
#include <Graphic/GraphicGraphicsPipeline.hpp>
#include <particle_frag.h>                   // for PARTICLE_FRAG
#include <particle_vert.h>                   // for PARTICLE_VERT
#include <struct/Particle.hpp>        // for Particle
#include <stdexcept>                         // for runtime_error
#include <vector>                            // for vector
#include <poike/poike.hpp>
// clang-format on

using namespace vkl;
using namespace poike;

GraphicGraphicsPipeline::GraphicGraphicsPipeline(const Device& device,
                                                 const SwapChain& swapChain,
                                                 const RenderPass& renderPass,
                                                 const DescriptorSetLayout& descriptorSetLayout)
    : GraphicsPipeline(device, swapChain, renderPass, descriptorSetLayout) {
  createPipeline();
}

GraphicGraphicsPipeline::~GraphicGraphicsPipeline() { destroyPipeline(); }

void GraphicGraphicsPipeline::createPipeline() {
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

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);

  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  VkPipelineViewportStateCreateInfo viewportState;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineMultisampleStateCreateInfo multisampling;
  VkPipelineColorBlendStateCreateInfo colorBlending;
  VkPipelineDepthStencilStateCreateInfo depthStencil;

  initDefaultPipeline<Particle>(vertexInputInfo, inputAssembly, viewportState, rasterizer, multisampling, colorBlending,
                                depthStencil);

  {
    inputAssembly.topology        = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    rasterizer.cullMode           = VK_CULL_MODE_NONE;
    depthStencil.depthTestEnable  = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp   = VK_COMPARE_OP_ALWAYS;

    // Additive blending
    VkPipelineColorBlendAttachmentState blendAttachmentState = {
        .blendEnable         = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .colorBlendOp        = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA,
        .alphaBlendOp        = VK_BLEND_OP_ADD,
        .colorWriteMask      = 0xF,
    };

    colorBlending = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &blendAttachmentState,
    };

    const VkShaderModule vertShaderModule = createShaderModule(PARTICLE_VERT);
    const VkShaderModule fragShaderModule = createShaderModule(PARTICLE_FRAG);

    shaderStages[0] = misc::pipelineShaderStageCreateInfo(vertShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = misc::pipelineShaderStageCreateInfo(fragShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);

    const VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount          = 2,
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depthStencil,
        .pColorBlendState    = &colorBlending,
        .pDynamicState       = nullptr,
        .layout              = m_layout,
        .renderPass          = m_renderPass.handle(),
        // Pipeline will be used in second sub pass
        .subpass            = 0,  // 1,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex  = -1,
    };

    if (vkCreateGraphicsPipelines(m_device.logical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline)
        != VK_SUCCESS) {
      throw std::runtime_error("Graphics Pipeline creation failed");
    }
  }

  deleteShaderModule(shaderStages);
}