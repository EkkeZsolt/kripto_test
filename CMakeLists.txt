cmake_minimum_required(VERSION 3.18)
project(LiptaiKripto LANGUAGES CXX CUDA)

# ──────────────────────────────────────────────
# C++ és CUDA szabványok
# ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# GPU architektúra – felhasználó állíthatja -DCMAKE_CUDA_ARCHITECTURES=XX
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 75)
endif()

# ──────────────────────────────────────────────
# Include útvonalak
# ──────────────────────────────────────────────
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/CGBN/include
)

# ──────────────────────────────────────────────
# Forrásfájlok
# ──────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/utils/BigIntConverter.cpp
    src/primality/TrialDivisionStrategy.cpp
    src/primality/MillerRabinGpuStrategy.cu
    src/search/PrimeSearcher.cpp
    src/search/PrimeSearchFactory.cpp
    src/observer/ConsoleLogger.cpp
    src/observer/ResultFileWriter.cpp
)

# ──────────────────────────────────────────────
# Fő executable
# ──────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:
        --expt-relaxed-constexpr
        -lineinfo
    >
)

# CUDA separable compilation (CGBN-hez szükséges)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

# ──────────────────────────────────────────────
# Tesztek (opcionális)
# ──────────────────────────────────────────────
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
