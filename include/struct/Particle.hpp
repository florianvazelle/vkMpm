#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <poike/poike.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

namespace vkm {
  // Pour plus de commentaire regarder Vertex.hpp
  struct alignas(16) Particle {
    alignas(16) glm::mat2 C;   // affine momentum matrix
    alignas(8) glm::vec2 pos;  // position "vec2" because this mpm example works in 2D
    alignas(8) glm::vec2 vel;  // velocity
    alignas(4) float mass;
    alignas(4) float volume_0;  // initial volume
    alignas(8) glm::vec2 padding;

    static VkVertexInputBindingDescription getBindingDescription() {
      // Binding description
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding   = 0;
      bindingDescription.stride    = sizeof(Particle);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      // Attribute descriptions
      // Describes memory layout and shader positions
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
          // position
          {
              .location = 0,
              .binding  = 0,
              .format   = VK_FORMAT_R32G32_SFLOAT,
              .offset   = offsetof(Particle, pos),
          },
          // velocity
          {
              .location = 1,
              .binding  = 0,
              .format   = VK_FORMAT_R32G32_SFLOAT,
              .offset   = offsetof(Particle, vel),
          },
          {
              .location = 2,
              .binding  = 0,
              .format   = VK_FORMAT_R32_SFLOAT,
              .offset   = offsetof(Particle, mass),
          },
      };

      return attributeDescriptions;
    }
  };

}  // namespace vkm

#endif  // PARTICLE_HPP