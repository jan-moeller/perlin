cmake_minimum_required(VERSION 3.12)
project(PerlinNoise)

set(CMAKE_CXX_STANDARD 17)

find_package(PNG REQUIRED)

add_executable(PerlinNoise main.cpp include/perlin_noise_generator.h include/point.h include/vector.h)
target_include_directories(PerlinNoise PUBLIC include)
target_link_libraries(PerlinNoise ${PNG_LIBRARY})