# Tesztek CMakeLists.txt

# Google Test letöltése FetchContent-tel
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ── BigInt Converter teszt ──
add_executable(test_bigint_converter
    test_bigint_converter.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/BigIntConverter.cpp
)
target_include_directories(test_bigint_converter PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/CGBN/include
)
target_link_libraries(test_bigint_converter GTest::gtest_main)
add_test(NAME BigIntConverterTest COMMAND test_bigint_converter)

# ── Primality teszt ──
add_executable(test_primality
    test_primality.cpp
    ${CMAKE_SOURCE_DIR}/src/primality/TrialDivisionStrategy.cpp
    ${CMAKE_SOURCE_DIR}/src/primality/MillerRabinGpuStrategy.cu
    ${CMAKE_SOURCE_DIR}/src/utils/BigIntConverter.cpp
)
target_include_directories(test_primality PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/CGBN/include
)
target_compile_options(test_primality PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:
        --expt-relaxed-constexpr
        -lineinfo
    >
)
set_target_properties(test_primality PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
)
target_link_libraries(test_primality GTest::gtest_main)
add_test(NAME PrimalityTest COMMAND test_primality)
