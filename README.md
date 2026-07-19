# High-Performance Cryptography Testbed

This repository contains a suite of high-performance cryptographic experiments and prime-searching tools optimized for modern hardware architectures, specifically targeting the **NVIDIA RTX 3090 GPU** and the **AMD Ryzen 9 5950X CPU**.

The project is divided into three distinct modules, each demonstrating a different approach to large integer arithmetic and primality testing.

## Modules

### 1. Miller-Rabin GPU Searcher (`/miller_rabin_searcher`)
An ultra-fast prime searcher that utilizes a deterministic Miller-Rabin algorithm. 
- **Architecture**: Employs an asynchronous CPU/GPU pipeline via CUDA Streams.
- **GPU Engine**: Uses **NVIDIA CGBN** (Cooperative Groups Big Number) to perform windowed Montgomery exponentiation concurrently on thousands of 512-bit (or up to 4096-bit) candidate numbers.
- **CPU Engine**: Uses OpenMP to pre-filter candidates utilizing all 32 threads of the Ryzen 9 5950X before shipping them to the VRAM via Pinned Memory.
- **Use Case**: Finding massive primes efficiently.

### 2. Trial Division GPU Searcher (`/trial_division_gpu_searcher`)
An experimental implementation of single-candidate primality testing.
- **Algorithm**: Tests a single candidate against a dynamically growing list of known primes stored directly in the GPU's VRAM.
- **Execution**: The CPU feeds a candidate, and the GPU launches a kernel where each CUDA thread tests the candidate against a distinct known prime using CGBN big-integer division. If no remainder is zero, the candidate is prime and added to the VRAM list.
- **Memory**: Capable of storing millions of large integer primes in the RTX 3090's 24GB GDDR6X VRAM.

### 3. RSA-129 Cracker Experiment (`/rsa_cracker_experiment`)
A practical demonstration of factoring a 129-digit RSA modulus (the famous Martin Gardner challenge).
- **Academic Approach**: Instead of using naive brute force, this experiment automates the download, extraction, and execution of **YAFU** (Yet Another Factoring Utility), integrating the academic GNFS (General Number Field Sieve) algorithm.
- **Performance**: Pushes the Ryzen 9 5950X to its absolute limits, deploying the workload across all 32 logical threads to factor the RSA-129 challenge in a realistic timeframe.

## Build Instructions

This project uses CMake as its build system. The NVIDIA CUDA Toolkit (v11.x or v12.x) and a compatible host compiler (MSVC 2019+ on Windows or GCC/Clang on Linux) are required.

### Dependencies
- **CUDA Toolkit**: Required for `miller_rabin_searcher` and `trial_division_gpu_searcher`.
- **CGBN**: Included as a header-only library in `external/CGBN`.
- **OpenMP**: Supported by your C++ compiler.

### Compiling
You can build the entire suite from the root directory:
```bash
# Generate build files
cmake -B build -S .

# Build specific targets in Release mode
cmake --build build --config Release --target MillerRabinSearcher
cmake --build build --config Release --target TrialDivisionGpuSearcher
cmake --build build --config Release --target RSACrackerExperiment
```

## Hardware Optimization Details
The code contains specific optimizations for SM 86 architecture (RTX 3090):
- **Occupancy**: Thread-blocks are sized at 256 to maximize warp scheduling on SM 86.
- **Math**: Compiled with `-use_fast_math` and PTX ISA optimizations (`--expt-relaxed-constexpr`).
- **Data Transfer**: Zero-copy pinned memory is utilized to hide H2D and D2H latency behind kernel execution in the Miller-Rabin searcher.

## License
MIT License.
