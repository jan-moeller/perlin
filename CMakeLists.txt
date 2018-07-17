cmake_minimum_required(VERSION 3.12)
project(perlin)

set(CMAKE_CXX_STANDARD 17)


add_library(noise INTERFACE)
target_sources(noise INTERFACE
        ${PROJECT_SOURCE_DIR}/include/seamless_noise_generator_2d.h
        ${PROJECT_SOURCE_DIR}/include/fractal_noise_generator.h
        ${PROJECT_SOURCE_DIR}/include/perlin_noise_generator.h
        ${PROJECT_SOURCE_DIR}/include/point.h
        ${PROJECT_SOURCE_DIR}/include/vector.h
        ${PROJECT_SOURCE_DIR}/include/math.h)
target_include_directories(noise INTERFACE include)

find_package(PNG)
if (PNG_FOUND)
    add_executable(noise_test main.cpp)
    target_link_libraries(noise_test ${PNG_LIBRARY} noise)
else()
    message(info "Did not find libpng. Not building test executable.")
endif ()
