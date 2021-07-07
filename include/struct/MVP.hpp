#ifndef UBO_HPP
#define UBO_HPP

#include <glm/glm.hpp>

namespace vkm {

  struct alignas(16) MVP {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

}  // namespace vkm

#endif  // UBO_HPP