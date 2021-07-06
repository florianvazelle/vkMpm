[![Actions Status](https://github.com/florianvazelle/vkMpm/workflows/build/badge.svg)](https://github.com/florianvazelle/vkMpm/actions)
[![Actions Status](https://github.com/florianvazelle/vkMpm/workflows/android/badge.svg)](https://github.com/florianvazelle/vkMpm/actions)
![Platform](https://img.shields.io/badge/platform-windows%20%7C%20linux%20%7C%20android-blue) 
[![License](https://img.shields.io/badge/license-Unlicense-blue.svg)](https://github.com/florianvazelle/vkMpm/blob/main/LICENSE)

# vkMpm

This is a Vulkan application to simulate 2D material with the [Material Point Method](https://en.wikipedia.org/wiki/Material_point_method).

There are two technical parts:
- Particle System in Vulkan
- Incremental MPM in Compute Shaders

## Building

### Build and run the standalone target

Use the following command to build and run the executable target.

```bash
cmake -Bbuild
cmake --build build
./build/bin/vkMpm --help
```

## Dependencies

- C++20 compiler :
  - Visual Studio 2019
  - GCC 9+ or Clang 10+
- [CMake](https://cmake.org/) for build system creation (>= 3.16.3)
- [Conan](https://conan.io/) for install packages (>= 1.0)

## References

- [nialltl/incremental_mpm](https://github.com/nialltl/incremental_mpm)

## Quote to save many hours

> Always check the alignment of structures