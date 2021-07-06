#ifndef COMPUTE_PARTICLE_HPP
#define COMPUTE_PARTICLE_HPP

namespace vkl {

  struct alignas(16) ComputeParticle {
    float deltaT;  // Frame delta time
    float particleCount;
    float elastic_lambda;
    float elastic_mu;
  };

}  // namespace vkl

#endif  // COMPUTE_PARTICLE_HPP