// clang-format off
#include <stdlib.h>                     // for EXIT_FAILURE, EXIT_SUCCESS
#include <cxxopts.hpp>                  // for OptionAdder, Options, ParseRe...
#include <iostream>                     // for operator<<, cout, endl, ostream
#include <memory>                       // for allocator, shared_ptr
#include <particle/ParticleSystem.hpp>  // for glfwInit, glfwTerminate, glfw...
#include <string>                       // for string
#include <common/Application.hpp>       // for DebugOption
// clang-format on

int main(int argc, char** argv) {
  cxxopts::Options options(argv[0], "A program to simulate a lava flow !");

  // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("d,debug", "Debug level (0: nothing, 1: error, 2: warning)", cxxopts::value<int>(), "LEVEL")
    ("e,error-exit", "Exit on first error");
  ;
  // clang-format on

  auto result = options.parse(argc, argv);

  if (result["help"].as<bool>()) {
    std::cout << options.help();
    return 0;
  }

  vkl::ParticleSystem::initialize();

  int debugLevel = 0;
  if (result.count("debug")) {
    debugLevel = result["debug"].as<int>();
  }

  vkl::DebugOption debugOption = {
      .debugLevel  = debugLevel,
      .exitOnError = result.count("error-exit") > 0,
  };

  vkl::ParticleSystem app("vkLavaMpm", debugOption);

  try {
    app.run();
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  vkl::ParticleSystem::terminate();

  return EXIT_SUCCESS;
}
