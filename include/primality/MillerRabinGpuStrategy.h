/***
 * MillerRabinGpuStrategy.h – GPU Miller-Rabin Strategy
 *
 * Design Pattern: Strategy (Concrete Strategy)
 * CGBN-alapú Miller-Rabin prímteszt GPU-n.
 * A CGBN Sample 4 mintájára, windowed Montgomery hatványozással.
 *
 * RTX 3090 optimalizált: CUDA Stream-es async pipeline támogatás.
 ***/

#pragma once
#include "primality/IPrimalityStrategy.h"
#include <cuda_runtime.h>
#include <cstdint>

class MillerRabinGpuStrategy : public IPrimalityStrategy {
public:
    /// @param miller_rabin_rounds Miller-Rabin iterációk száma (több = pontosabb)
    /// @param threads_per_block   GPU threads per block
    explicit MillerRabinGpuStrategy(uint32_t miller_rabin_rounds = 20,
                                     uint32_t threads_per_block = 256);

    ~MillerRabinGpuStrategy() override;

    std::string name() const override;

    /// Szinkron batch teszt (kompatibilitás)
    std::vector<PrimalityResult> testBatch(
        const std::vector<std::vector<uint32_t>>& candidates,
        uint32_t bit_length) override;

    /// Aszinkron kernel launch – nem blokkolja a CPU-t
    /// @return A host results puffer (pinned memória), amely a syncResults() után tölti fel
    void launchAsync(
        const std::vector<std::vector<uint32_t>>& candidates,
        uint32_t bit_length);

    /// Várakozás az aszinkron kernel befejezésére és eredmények visszaadása
    std::vector<PrimalityResult> syncResults();

private:
    uint32_t mr_rounds_;
    uint32_t tpb_;
    cudaStream_t stream_ = nullptr;

    // Async állapot: az utolsó launchAsync hívás adatai
    uint32_t async_instance_count_ = 0;
    uint32_t async_num_rounds_ = 0;

    /// Miller-Rabin bázis prímek generálása (mindig determinisztikus az összes tanúval)
    std::vector<uint32_t> generateDeterministicWitnesses(uint32_t bit_length) const;
};
