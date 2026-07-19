cmake_minimum_required(VERSION 3.18)
project(RSACrackerExperiment LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 86)
endif()

# We need CGBN for handling the 129-digit number (which is 428 bits, fits in 512 bits)
include_directories(${CMAKE_SOURCE_DIR}/external/CGBN/include)

add_executable(${PROJECT_NAME} main.cpp)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:
        --expt-relaxed-constexpr
        --use_fast_math
        -O3
    >
)

if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE /LTCG)
endif()
