#ifndef PARTICLEMVP_HPP
#define PARTICLEMVP_HPP

#include <common/struct/MVP.hpp>
#include <glm/glm.hpp>

namespace vkl {

  struct ParticleMVP : public MVP {
    glm::vec2 screenDim;
  };

}  // namespace vkl

#endif  // PARTICLEMVP_HPP
