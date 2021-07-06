#pragma once

#include <time.h>
#include <common/CommandBuffers.hpp>
#include <common/CommandPool.hpp>
#include <common/Device.hpp>
#include <common/buffer/Buffer.hpp>
#include <common/buffer/StorageBuffer.hpp>
#include <common/struct/Cell.hpp>
#include <common/struct/Particle.hpp>

#include <random>
#include <vector>

#define NUM_PARTICLE 4096
#define GRID_RESOLUTION 64
#define NUM_CELLS (GRID_RESOLUTION * GRID_RESOLUTION)
#define ELASTIC_LAMBDA 10.0f
#define ELASTIC_MU 20.0f
#define DT 0.1f

namespace vkl {

  static float elastic_lambda = ELASTIC_LAMBDA;
  static float elastic_mu     = ELASTIC_MU;

  class MPMStorageBuffer {
  public:
    StorageBuffer ps;
    StorageBuffer grid;
    StorageBuffer fs;

    MPMStorageBuffer(const Device& device,
                     const CommandPool& commandPool,
                     VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags properties)
        : m_device(device),
          m_commandPool(commandPool),
          ps(device, NUM_PARTICLE * sizeof(Particle), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | usage, properties),
          grid(device, NUM_CELLS * sizeof(Cell), usage, properties),
          fs(device, NUM_PARTICLE * sizeof(glm::mat2), usage, properties) {
      createMPMStorageBuffer();
    }

    void createMPMStorageBuffer() {
      // STEP 1 - we populate our array of particles

      std::vector<glm::vec2> temp_positions;

      const int x     = GRID_RESOLUTION / 2;
      const int y     = GRID_RESOLUTION / 2;
      const int box_x = 32;
      const int box_y = 32;

      const float spacing = 0.5f;
      for (float i = -box_x / 2; i < box_x / 2; i += spacing) {
        for (float j = -box_y / 2; j < box_y / 2; j += spacing) {
          temp_positions.push_back(glm::vec2(x + i, y + j));
        }
      }

      std::vector<Particle> particleBuffer(NUM_PARTICLE);
      std::vector<glm::mat2> FsBuffer(NUM_PARTICLE);

      // STEP 2 - initialise particles

      for (int i = 0; i < NUM_PARTICLE; ++i) {
        particleBuffer[i] = {
            .C    = glm::mat2(0, 0, 0, 0),
            .pos  = temp_positions[i],
            .vel  = glm::vec2(0, 0),
            .mass = 1.0f,
        };

        // deformation gradient initialised to the identity
        FsBuffer[i] = glm::mat2(1.0f);
      }

      std::vector<Cell> gridBuffer(NUM_CELLS);

      for (int i = 0; i < NUM_CELLS; ++i) {
        gridBuffer[i] = {
            .vel = glm::vec2(0, 0),
        };
      }

      // MPM course, equation 152

      // STEP 3 - launch a P2G job to scatter particle mass to the grid

      Job_P2G(particleBuffer, FsBuffer, gridBuffer);

      std::vector<glm::vec2> weights(3);

      for (int i = 0; i < NUM_PARTICLE; ++i) {
        Particle& p = particleBuffer[i];

        // quadratic interpolation weights
        glm::vec2 cell_idx  = glm::ivec2(p.pos);
        glm::vec2 cell_diff = (p.pos - cell_idx) - 0.5f;
        weights[0]          = 0.5f * ((0.5f - cell_diff) * (0.5f - cell_diff));
        weights[1]          = 0.75f - (cell_diff * cell_diff);
        weights[2]          = 0.5f * ((0.5f + cell_diff) * (0.5f + cell_diff));

        float density = 0.0f;
        // iterate over neighbouring 3x3 cells
        for (int gx = 0; gx < 3; ++gx) {
          for (int gy = 0; gy < 3; ++gy) {
            float weight = weights[gx].x * weights[gy].y;

            // map 2D to 1D index in grid
            int cell_index = (cell_idx.x + (gx - 1)) * GRID_RESOLUTION + (cell_idx.y + gy - 1);
            density += gridBuffer[cell_index].mass * weight;
          }
        }

        // per-particle volume estimate has now been computed
        float volume = p.mass / density;
        p.volume_0   = volume;
      }

      // ----- Copy particle buffer -----

      CopyVertexBuffer(m_device, m_commandPool, particleBuffer, ps);
      CopyBuffer(m_device, m_commandPool, gridBuffer, grid);
      CopyBuffer(m_device, m_commandPool, FsBuffer, fs);
    }

    void recreate() { createMPMStorageBuffer(); }

  private:
    const Device& m_device;
    const CommandPool& m_commandPool;

    // TODO : maybe use Compute Shader
    void Job_P2G(const std::vector<Particle>& particleBuffer,
                 const std::vector<glm::mat2>& Fs,
                 std::vector<Cell>& grid) const {
      std::vector<glm::vec2> weights(3);

      for (int i = 0; i < NUM_PARTICLE; ++i) {
        const Particle& p = particleBuffer[i];

        glm::mat2 stress = glm::mat2(0, 0, 0, 0);

        // deformation gradient
        const glm::mat2& F = Fs[i];

        float J = glm::determinant(F);

        // MPM course, page 46
        float volume = p.volume_0 * J;

        // useful matrices for Neo-Hookean model
        glm::mat2 F_T             = glm::transpose(F);
        glm::mat2 F_inv_T         = glm::inverse(F_T);
        glm::mat2 F_minus_F_inv_T = F - F_inv_T;

        // MPM course equation 48
        glm::mat2 P_term_0 = elastic_mu * (F_minus_F_inv_T);
        glm::mat2 P_term_1 = elastic_lambda * glm::log(J) * F_inv_T;
        glm::mat2 P        = P_term_0 + P_term_1;

        // cauchy_stress = (1 / det(F)) * P * F_T
        // equation 38, MPM course
        stress = (1.0f / J) * (P * F_T);

        // (M_p)^-1 = 4, see APIC paper and MPM course page 42
        // this term is used in MLS-MPM paper eq. 16. with quadratic weights, Mp = (1/4) * (delta_x)^2.
        // in this simulation, delta_x = 1, because i scale the rendering of the domain rather than the domain itself.
        // we multiply by dt as part of the process of fusing the momentum and force update for MLS-MPM
        glm::mat2 eq_16_term_0 = -volume * 4 * stress * DT;

        // quadratic interpolation weights
        glm::vec2 cell_idx  = glm::ivec2(p.pos);  // uvec2 -> unsigned
        glm::vec2 cell_diff = (p.pos - cell_idx) - 0.5f;
        weights[0]          = 0.5f * ((0.5f - cell_diff) * (0.5f - cell_diff));
        weights[1]          = 0.75f - (cell_diff * cell_diff);
        weights[2]          = 0.5f * ((0.5f + cell_diff) * (0.5f + cell_diff));

        // for all surrounding 9 cells
        for (unsigned int gx = 0; gx < 3; ++gx) {
          for (unsigned int gy = 0; gy < 3; ++gy) {
            float weight = weights[gx].x * weights[gy].y;

            glm::vec2 cell_x    = glm::ivec2(cell_idx.x + gx - 1, cell_idx.y + gy - 1);  // uint2
            glm::vec2 cell_dist = (cell_x - p.pos) + 0.5f;                               // cast uvec2 into vec2
            glm::vec2 Q         = p.C * cell_dist;

            // scatter mass and momentum to the grid
            int cell_index = cell_x.x * GRID_RESOLUTION + cell_x.y;
            Cell& cell     = grid[cell_index];

            // MPM course, equation 172
            float weighted_mass = weight * p.mass;
            cell.mass += weighted_mass;

            // APIC P2G momentum contribution
            cell.vel += weighted_mass * (p.vel + Q);

            // fused force/momentum update from MLS-MPM
            // see MLS-MPM paper, equation listed after eqn. 28
            glm::vec2 momentum = (eq_16_term_0 * weight) * cell_dist;
            cell.vel += momentum;
          }
        }
      }
    }

    template <typename T> void CopyBuffer(const Device& device,
                                          const CommandPool& commandPool,
                                          const std::vector<T>& vec,
                                          StorageBuffer& storageBuffer) const {
      Buffer<T> stagingBuffer(device, vec, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

      CommandBuffers::SingleTimeCommands(
          device, commandPool, device.graphicsQueue(), [&](const VkCommandBuffer& cmdBuffer) {
            VkBufferCopy copyRegion = {
                .size = storageBuffer.size(),
            };
            vkCmdCopyBuffer(cmdBuffer, stagingBuffer.buffer(), storageBuffer.buffer(), 1, &copyRegion);

            vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
                                 0, nullptr, 0, nullptr, 0, nullptr);
          });
    }

    template <typename T> void CopyVertexBuffer(const Device& device,
                                                const CommandPool& commandPool,
                                                const std::vector<T>& vec,
                                                StorageBuffer& storageBuffer) const {
      Buffer<T> stagingBuffer(device, vec, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

      CommandBuffers::SingleTimeCommands(
          device, commandPool, device.graphicsQueue(), [&](const VkCommandBuffer& cmdBuffer) {
            VkBufferCopy copyRegion = {
                .size = storageBuffer.size(),
            };
            vkCmdCopyBuffer(cmdBuffer, stagingBuffer.buffer(), storageBuffer.buffer(), 1, &copyRegion);

            const std::optional<uint32_t>& graphicsFamily = device.queueFamilyIndices().graphicsFamily;
            const std::optional<uint32_t>& computeFamily  = device.queueFamilyIndices().computeFamily;

            if (graphicsFamily.value() != computeFamily.value()) {
              const VkBufferMemoryBarrier buffer_barrier = {
                  .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                  .srcAccessMask       = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                  .dstAccessMask       = 0,
                  .srcQueueFamilyIndex = graphicsFamily.value(),
                  .dstQueueFamilyIndex = computeFamily.value(),
                  .buffer              = storageBuffer.buffer(),
                  .offset              = 0,
                  .size                = storageBuffer.size(),
              };

              vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                   0, 0, nullptr, 1, &buffer_barrier, 0, nullptr);
            }
          });
    }
  };
}  // namespace vkl
