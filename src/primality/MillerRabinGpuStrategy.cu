/***
 * MillerRabinGpuStrategy.cu – GPU Miller-Rabin CGBN Implementáció
 *
 * A CGBN Sample 4 (miller_rabin.cu) mintájára építve.
 * Windowed Montgomery hatványozást használ.
 ***/

#include "primality/MillerRabinGpuStrategy.h"
#include "cuda/CudaMemory.h"
#include "config/CgbnConfig.h"
#include <cgbn/cgbn.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <algorithm>

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) \
        throw std::runtime_error(std::string("CUDA error: ") + cudaGetErrorString(err)); \
} while(0)

// ════════════════════════════════════════════════
// Miller-Rabin CGBN Device Class
// ════════════════════════════════════════════════
template<class params>
class MillerRabinDevice {
public:
    static const uint32_t window_bits = params::WINDOW_BITS;

    typedef struct {
        cgbn_mem_t<params::BITS> candidate;
        uint32_t                 passed;
    } instance_t;

    typedef cgbn_context_t<params::TPI, params>    context_t;
    typedef cgbn_env_t<context_t, params::BITS>    env_t;
    typedef typename env_t::cgbn_t                 bn_t;
    typedef typename env_t::cgbn_local_t           bn_local_t;
    typedef typename env_t::cgbn_wide_t            bn_wide_t;

    context_t _context;
    env_t     _env;
    int32_t   _instance;

    __device__ __forceinline__
    MillerRabinDevice(cgbn_monitor_t monitor, cgbn_error_report_t *report, int32_t instance)
        : _context(monitor, report, (uint32_t)instance), _env(_context), _instance(instance) {}

    __device__ __forceinline__
    void powm(bn_t &x, const bn_t &power, const bn_t &modulus) {
        bn_t       t;
        bn_local_t window[1 << window_bits];
        int32_t    index, position, offset;
        uint32_t   np0;

        cgbn_negate(_env, t, modulus);
        cgbn_store(_env, window + 0, t);
        np0 = cgbn_bn2mont(_env, x, x, modulus);
        cgbn_store(_env, window + 1, x);
        cgbn_set(_env, t, x);

        #pragma nounroll
        for (index = 2; index < (1 << window_bits); index++) {
            cgbn_mont_mul(_env, x, x, t, modulus, np0);
            cgbn_store(_env, window + index, x);
        }

        position = params::BITS - cgbn_clz(_env, power);
        offset = position % window_bits;
        if (offset == 0) position -= window_bits;
        else             position -= offset;
        index = cgbn_extract_bits_ui32(_env, power, position, window_bits);
        cgbn_load(_env, x, window + index);

        while (position > 0) {
            #pragma nounroll
            for (int s = 0; s < window_bits; s++)
                cgbn_mont_sqr(_env, x, x, modulus, np0);
            position -= window_bits;
            index = cgbn_extract_bits_ui32(_env, power, position, window_bits);
            cgbn_load(_env, t, window + index);
            cgbn_mont_mul(_env, x, x, t, modulus, np0);
        }
        cgbn_mont2bn(_env, x, x, modulus, np0);
    }

    __device__ __forceinline__
    uint32_t miller_rabin_test(const bn_t &candidate, uint32_t *primes, uint32_t prime_count) {
        int       k, trailing, count;
        bn_t      x, power, minus_one;
        bn_wide_t w;

        cgbn_sub_ui32(_env, power, candidate, 1);
        trailing = cgbn_ctz(_env, power);
        cgbn_rotate_right(_env, power, power, trailing);

        for (k = 0; k < (int)prime_count; k++) {
            cgbn_set_ui32(_env, x, primes[k]);
            powm(x, power, candidate);
            cgbn_sub_ui32(_env, minus_one, candidate, 1);
            if (!cgbn_equals_ui32(_env, x, 1) && !cgbn_equals(_env, x, minus_one)) {
                if (trailing == 1) return k;
                count = trailing;
                while (true) {
                    cgbn_sqr_wide(_env, w, x);
                    cgbn_rem_wide(_env, x, w, candidate);
                    if (cgbn_equals(_env, x, minus_one)) break;
                    if (--count == 0 || cgbn_equals_ui32(_env, x, 1)) return k;
                }
            }
        }
        return prime_count;
    }
};

// ════════════════════════════════════════════════
// GPU Kernel
// ════════════════════════════════════════════════
template<class params>
__global__ void kernel_miller_rabin(
        cgbn_error_report_t *report,
        typename MillerRabinDevice<params>::instance_t *instances,
        uint32_t instance_count,
        uint32_t *primes, uint32_t prime_count) {
    int32_t inst = (blockIdx.x * blockDim.x + threadIdx.x) / params::TPI;
    if (inst >= (int32_t)instance_count) return;

    typedef MillerRabinDevice<params> mr_t;
    mr_t                     mr(cgbn_report_monitor, report, inst);
    typename mr_t::bn_t      candidate;
    cgbn_load(mr._env, candidate, &(instances[inst].candidate));
    instances[inst].passed = mr.miller_rabin_test(candidate, primes, prime_count);
}

// ════════════════════════════════════════════════
// Host Strategy Implementation
// ════════════════════════════════════════════════
MillerRabinGpuStrategy::MillerRabinGpuStrategy(uint32_t rounds, uint32_t tpb)
    : mr_rounds_(rounds), tpb_(tpb) {}

MillerRabinGpuStrategy::~MillerRabinGpuStrategy() = default;

std::string MillerRabinGpuStrategy::name() const {
    return "MillerRabin-GPU (CGBN 4096-bit)";
}

std::vector<uint32_t> MillerRabinGpuStrategy::generateWitnessPrimes(uint32_t count) const {
    std::vector<uint32_t> primes;
    primes.push_back(2);
    uint32_t cur = 3;
    while (primes.size() < count) {
        bool ok = true;
        for (auto p : primes) { if (cur % p == 0) { ok = false; break; } }
        if (ok) primes.push_back(cur);
        cur += 2;
    }
    return primes;
}

std::vector<PrimalityResult> MillerRabinGpuStrategy::testBatch(
        const std::vector<std::vector<uint32_t>>& candidates, uint32_t bit_length) {
    using params = DefaultParams;
    using instance_t = typename MillerRabinDevice<params>::instance_t;

    const uint32_t ic = (uint32_t)candidates.size();
    const uint32_t nl = params::BITS / 32;
    const int32_t  TPB = tpb_ ? tpb_ : 128;
    const int32_t  IPB = TPB / params::TPI;

    auto witness = generateWitnessPrimes(mr_rounds_);

    std::vector<instance_t> host(ic);
    for (uint32_t i = 0; i < ic; i++) {
        std::memset(&host[i].candidate, 0, sizeof(cgbn_mem_t<params::BITS>));
        uint32_t cl = std::min((uint32_t)candidates[i].size(), nl);
        std::memcpy(host[i].candidate._limbs, candidates[i].data(), sizeof(uint32_t)*cl);
        host[i].passed = 0;
    }

    CudaMemory<instance_t> gpu_inst(ic);
    CudaMemory<uint32_t>   gpu_pr(mr_rounds_);
    gpu_inst.copyToDevice(host.data());
    gpu_pr.copyToDevice(witness.data());

    cgbn_error_report_t *report;
    CUDA_CHECK(cgbn_error_report_alloc(&report));

    kernel_miller_rabin<params><<<(ic+IPB-1)/IPB, TPB>>>(
        report, gpu_inst.get(), ic, gpu_pr.get(), mr_rounds_);
    CUDA_CHECK(cudaDeviceSynchronize());

    if (cgbn_error_report_check(report)) {
        cgbn_error_report_free(report);
        throw std::runtime_error("CGBN error in Miller-Rabin kernel");
    }

    gpu_inst.copyToHost(host.data());
    cgbn_error_report_free(report);

    std::vector<PrimalityResult> res;
    res.reserve(ic);
    for (uint32_t i = 0; i < ic; i++) {
        res.push_back({i, host[i].passed == mr_rounds_, host[i].passed});
    }
    return res;
}
