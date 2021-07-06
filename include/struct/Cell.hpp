#ifndef CELL_HPP
#define CELL_HPP

#include <glm/glm.hpp>

namespace vkl {

  struct alignas(16) Cell {
    alignas(8) glm::vec2 vel;  // velocity
    alignas(4) float mass;
    alignas(4) float padding;
  };

}  // namespace vkl

#endif  // CELL_HPP
