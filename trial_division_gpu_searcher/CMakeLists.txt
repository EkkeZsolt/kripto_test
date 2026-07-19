cmake_minimum_required(VERSION 3.18)
project(TrialDivisionGpuSearcher LANGUAGES CXX CUDA)

# ──────────────────────────────────────────────
# C++ és CUDA szabványok
# ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# GPU architektúra – RTX 3090 = SM 86 (Ampere)
# Felhasználó állíthatja -DCMAKE_CUDA_ARCHITECTURES=XX
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 86)
endif()

# ──────────────────────────────────────────────
# OpenMP – Ryzen 9 5950X összes magjának kihasználásához
cmake_minimum_required(VERSION 3.18)
project(TrialDivisionGpuSearcher LANGUAGES CXX CUDA)

# ──────────────────────────────────────────────
# C++ és CUDA szabványok
# ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# GPU architektúra – RTX 3090 = SM 86 (Ampere)
# Felhasználó állíthatja -DCMAKE_CUDA_ARCHITECTURES=XX
if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
    set(CMAKE_CUDA_ARCHITECTURES 86)
endif()

# ──────────────────────────────────────────────
# OpenMP – Ryzen 9 5950X összes magjának kihasználásához
# ──────────────────────────────────────────────
find_package(OpenMP REQUIRED COMPONENTS CXX)

# ──────────────────────────────────────────────
# Include útvonalak
# ──────────────────────────────────────────────
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/CGBN/include
)

# ──────────────────────────────────────────────
# Forrásfájlok
# ──────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/utils/BigIntConverter.cpp
    src/primality/TrialDivisionStrategy.cpp
    src/primality/GpuTrialDivisionStrategy.cu
    src/search/PrimeSearcher.cpp
    src/search/PrimeSearchFactory.cpp
    src/observer/ConsoleLogger.cpp
    src/observer/ResultFileWriter.cpp
)

# ──────────────────────────────────────────────
# Fő executable
# ──────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${SOURCES})

# ── CUDA Optimalizációs Flag-ek ──
# --use_fast_math: gyorsabb matematikai műveletek (fp32)
# -O3: maximális CUDA optimalizáció
# --expt-relaxed-constexpr: CGBN-hez szükséges
# -lineinfo: profiling támogatás (minimális overhead)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:
        --expt-relaxed-constexpr
        --use_fast_math
        -O3
        -lineinfo
        --threads=0
    >
)

# ── C++ Optimalizációs Flag-ek ──
# MSVC: /O2 /Oi (Release-ben automatikus, de explicit is beállítjuk)
# GCC/Clang: -O3 -march=znver3 (Zen 3 = Ryzen 5950X natív)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:MSVC>>:
        /O2 /Oi /GL
    >
    $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:
        -O3 -march=znver3
    >
)

# ── MSVC link-time optimalizáció ──
if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE /LTCG)
endif()

# ── OpenMP linkek ──
target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)

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
