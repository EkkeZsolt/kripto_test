#include "primality/GpuTrialDivisionStrategy.h"
#include <iostream>
#include <cuda_runtime.h>
#include <cgbn/cgbn.h>

#define TPI 16
#define BITS 512

typedef cgbn_context_t<TPI>         context_t;
typedef cgbn_env_t<context_t, BITS> env_t;

// CGBN hibakezelés (üres a maximális sebességért, vagy minimális)
__device__ cgbn_error_report_t* g_report;

__global__ void trial_division_kernel(
    uint32_t* primes_array,
    uint32_t* candidate_array,
    size_t num_primes,
    uint32_t* is_composite_flag)
{
    context_t      bn_context(cgbn_report_monitor, g_report, 0);
    env_t          bn_env(bn_context);
    env_t::cgbn_t  r_candidate, r_prime, r_rem;

    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    int prime_idx = tid / TPI;
    
    if (prime_idx >= num_primes) return;

    // Load candidate (same for all)
    cgbn_load(bn_env, r_candidate, (cgbn_mem_t<BITS>*)candidate_array);
    
    // Load prime for this instance
    cgbn_load(bn_env, r_prime, (cgbn_mem_t<BITS>*)(primes_array + prime_idx * (BITS / 32)));

    // Remainder: r_rem = r_candidate % r_prime
    cgbn_rem(bn_env, r_rem, r_candidate, r_prime);

    // If remainder is 0, it's composite
    if (cgbn_compare_ui32(bn_env, r_rem, 0) == 0) {
        *is_composite_flag = 1;
    }
}

GpuTrialDivisionStrategy::GpuTrialDivisionStrategy() {
    d_primes_count = 0;
    d_primes_capacity = 1000000; // Start with capacity for 1 million primes
    cudaMalloc(&d_known_primes, d_primes_capacity * (BITS / 32) * sizeof(uint32_t));
}

GpuTrialDivisionStrategy::~GpuTrialDivisionStrategy() {
    cudaFree(d_known_primes);
}

std::string GpuTrialDivisionStrategy::name() const {
    return "GpuTrialDivision-Single";
}

void GpuTrialDivisionStrategy::addKnownPrime(const std::vector<uint32_t>& prime_limbs) {
    if (d_primes_count >= d_primes_capacity) {
        // Reallocate
        size_t new_cap = d_primes_capacity * 2;
        uint32_t* new_ptr;
        cudaMalloc(&new_ptr, new_cap * (BITS / 32) * sizeof(uint32_t));
        cudaMemcpy(new_ptr, d_known_primes, d_primes_count * (BITS / 32) * sizeof(uint32_t), cudaMemcpyDeviceToDevice);
        cudaFree(d_known_primes);
        d_known_primes = new_ptr;
        d_primes_capacity = new_cap;
    }

    std::vector<uint32_t> padded = prime_limbs;
    padded.resize(BITS / 32, 0);

    cudaMemcpy(d_known_primes + d_primes_count * (BITS / 32), padded.data(), (BITS / 32) * sizeof(uint32_t), cudaMemcpyHostToDevice);
    d_primes_count++;
}

bool GpuTrialDivisionStrategy::testSingleCandidate(
    const std::vector<uint32_t>& candidate,
    uint32_t bit_length)
{
    if (d_primes_count == 0) return true; // No primes to divide by yet

    std::vector<uint32_t> padded = candidate;
    padded.resize(BITS / 32, 0);

    uint32_t* d_candidate;
    cudaMalloc(&d_candidate, (BITS / 32) * sizeof(uint32_t));
    cudaMemcpy(d_candidate, padded.data(), (BITS / 32) * sizeof(uint32_t), cudaMemcpyHostToDevice);

    uint32_t* d_is_composite;
    cudaMalloc(&d_is_composite, sizeof(uint32_t));
    uint32_t is_composite = 0;
    cudaMemcpy(d_is_composite, &is_composite, sizeof(uint32_t), cudaMemcpyHostToDevice);

    int threads_per_block = 256;
    int primes_per_block = threads_per_block / TPI;
    int num_blocks = (d_primes_count + primes_per_block - 1) / primes_per_block;

    trial_division_kernel<<<num_blocks, threads_per_block>>>(
        d_known_primes,
        d_candidate,
        d_primes_count,
        d_is_composite
    );
    cudaDeviceSynchronize();

    cudaMemcpy(&is_composite, d_is_composite, sizeof(uint32_t), cudaMemcpyDeviceToHost);

    cudaFree(d_candidate);
    cudaFree(d_is_composite);

    return (is_composite == 0); // true if not composite (i.e. prime)
}
