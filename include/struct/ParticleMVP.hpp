#ifndef PARTICLEMVP_HPP
#define PARTICLEMVP_HPP

#include <struct/MVP.hpp>
#include <glm/glm.hpp>

namespace vkm {

  struct ParticleMVP : public MVP {
    glm::vec2 screenDim;
  };

}  // namespace vkm

#endif  // PARTICLEMVP_HPP
