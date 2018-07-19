cmake_minimum_required(VERSION 3.12)
project(perlin)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

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
