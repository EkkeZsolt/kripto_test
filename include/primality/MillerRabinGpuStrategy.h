/***
 * MillerRabinGpuStrategy.h – GPU Miller-Rabin Strategy
 *
 * Design Pattern: Strategy (Concrete Strategy)
 * CGBN-alapú Miller-Rabin prímteszt GPU-n.
 * A CGBN Sample 4 mintájára, windowed Montgomery hatványozással.
 ***/

#pragma once
#include "primality/IPrimalityStrategy.h"
#include <cstdint>

class MillerRabinGpuStrategy : public IPrimalityStrategy {
public:
    /// @param miller_rabin_rounds Miller-Rabin iterációk száma (több = pontosabb)
    /// @param threads_per_block   GPU threads per block
    explicit MillerRabinGpuStrategy(uint32_t miller_rabin_rounds = 20,
                                     uint32_t threads_per_block = 128);

    ~MillerRabinGpuStrategy() override;

    std::string name() const override;

    std::vector<PrimalityResult> testBatch(
        const std::vector<std::vector<uint32_t>>& candidates,
        uint32_t bit_length) override;

private:
    uint32_t mr_rounds_;
    uint32_t tpb_;

    /// Miller-Rabin bázis prímek generálása (mindig determinisztikus az összes tanúval)
    std::vector<uint32_t> generateDeterministicWitnesses(uint32_t bit_length) const;
};
