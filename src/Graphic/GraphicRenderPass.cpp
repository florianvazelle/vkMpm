#include <Graphic/GraphicRenderPass.hpp>
#include <stddef.h>
#include <stdint.h>
#include <array>
#include <memory>
#include <poike/poike.hpp>
#include <stdexcept>
#include <vector>

using namespace poike;
using namespace vkl;

GraphicRenderPass::GraphicRenderPass(const Device& device, const SwapChain& swapChain) : RenderPass(device, swapChain) {
  createRenderPass();
  createFrameBuffers();
}

void GraphicRenderPass::createRenderPass() {
  /**
   * Note Exposé : Les renderpasses permettent d'informer Vulkan des attachements des framebuffers utilisés lors du
   * rendu. Nous devons indiquer combien chaque framebuffer aura de buffers de couleur et de profondeur, combien de
   * samples il faudra utiliser avec chaque frambuffer et comment les utiliser tout au long des opérations de rendu.
   */

  // const VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
  const VkFormat depthFormat = misc::findDepthFormat(m_device.physical());

  /* STEP 1 : Description des attachments */

  // Create a new render pass as a color attachment
  const VkAttachmentDescription colorAttachment = {
      // Format should match the format of the swap chain
      .format = m_swapChain.imageFormat(),
      // No multisampling
      .samples = VK_SAMPLE_COUNT_1_BIT,
      // Clear data before rendering, then store result after
      .loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      // Not doing anything with stencils, so don't care about it
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      // Don't care about initial layout, but final layout should
      // be same as the presentation source
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  // Create a new render pass as a depth attachment
  const VkAttachmentDescription depthAttachment = {
      .format         = depthFormat,
      .samples        = VK_SAMPLE_COUNT_1_BIT,
      .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,  // Attachment will be transitioned to
                                                                           // shader read at render pass end,
  };

  /**
   * Note Exposé : Les subpasses sont des opérations de rendu dépendant du contenu présent dans le
   * framebuffer quand elles commencent. Elles peuvent consister en des opérations de
   * post-processing exécutées l'une après l'autre. En regroupant toutes ces opérations en une seule
   * passe, Vulkan peut alors réaliser des optimisations et conserver de la bande passante pour de
   * potentiellement meilleures performances.
   */

  /* STEP 2 : les references des attachments (utilisé pour décrire les subpass) */

  // Depth for the depth_basic shader (fisrt subpass)
  const VkAttachmentReference depthRef = {
      .attachment = 1,
      // Use the optimal layout for depth attachments
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  };

  // Swapchain's color for the shadow_mapping shader (second subpass)
  const VkAttachmentReference colorRef = {
      // Index of 0 in color attachments description array (we're only using 1)
      .attachment = 0,
      // Use the optimal layout for color attachments
      .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  // On définie ici l'attachments du depth mais pour le shadow_mapping shader (second subpass)
  // mais ici en lecture exclusive
  // const VkAttachmentReference inputRef = {
  //     .attachment = 1,
  //     .layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  // };

  /* STEP 3 : les subpass */

  // First subpass
  // Fill depth attachments
  // const VkSubpassDescription depthSubpass = {
  //     .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
  //     .colorAttachmentCount    = 0,
  //     .pDepthStencilAttachment = &depthRef,
  // };

  // Second subpass
  // Default subpass (read the input attachment (of the previous subpass) and write the swapchain's color attachment)
  const VkSubpassDescription colorSubpass = {
      // Using for graphics computation
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      // Use the attachments filled in the first pass as input attachments
      // .inputAttachmentCount = 1,
      // .pInputAttachments    = &inputRef,
      .colorAttachmentCount    = 1,
      .pColorAttachments       = &colorRef,
      .pDepthStencilAttachment = &depthRef,
  };

  /* STEP 4 : Les dependances des subpass */

  std::array<VkSubpassDependency, 2> dependencies;

  dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass      = 0;
  dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[1].srcSubpass      = 0;
  dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
  dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  /* STEP 5 : Création de la render pass */

  const std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};
  const std::vector<VkSubpassDescription> subpass        = {/*depthSubpass, */ colorSubpass};
  // const std::vector<VkSubpassDependency> dependencies = {defaultDependency, depthDependencies[0],
  // depthDependencies[1]};

  const VkRenderPassCreateInfo createInfo = {
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = static_cast<uint32_t>(attachments.size()),
      .pAttachments    = attachments.data(),
      .subpassCount    = static_cast<uint32_t>(subpass.size()),
      .pSubpasses      = subpass.data(),
      .dependencyCount = static_cast<uint32_t>(dependencies.size()),
      .pDependencies   = dependencies.data(),
  };

  if (vkCreateRenderPass(m_device.logical(), &createInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
    throw std::runtime_error("Render pass creation failed");
  }
}

void GraphicRenderPass::createFrameBuffers() {
  const size_t numImages     = m_swapChain.numImages();
  const VkFormat depthFormat = misc::findDepthFormat(m_device.physical());

  // Fill attachments for one depth attachment by frame
  m_depthAttachments.resize(numImages);
  for (size_t i = 0; i < numImages; i++) {
    m_depthAttachments[i] = std::make_unique<Attachment>(m_device, m_swapChain, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }

  m_frameBuffers.resize(numImages);

  // Initialize default framebuffer creation info
  VkImageView attachments[2];
  const VkFramebufferCreateInfo info = {
      .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass      = m_renderPass,
      .attachmentCount = 2,
      .pAttachments    = attachments,
      .width           = m_swapChain.extent().width,
      .height          = m_swapChain.extent().height,
      .layers          = 1,
  };

  // Create a framebuffer for each image view
  for (size_t i = 0; i < numImages; ++i) {
    attachments[0] = m_swapChain.imageView(i);
    attachments[1] = m_depthAttachments[i]->view();

    if (vkCreateFramebuffer(m_device.logical(), &info, nullptr, &m_frameBuffers.at(i)) != VK_SUCCESS) {
      throw std::runtime_error("Framebuffer creation failed");
    }
  }
}
