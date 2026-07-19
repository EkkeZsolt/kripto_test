#pragma once
#include "primality/IPrimalityStrategy.h"
#include <vector>

class GpuTrialDivisionStrategy : public IPrimalityStrategy {
private:
    uint32_t* d_known_primes;
    size_t d_primes_capacity;
    size_t d_primes_count;

public:
    GpuTrialDivisionStrategy();
    ~GpuTrialDivisionStrategy() override;

    std::string name() const override;

    void addKnownPrime(const std::vector<uint32_t>& prime_limbs) override;

    bool testSingleCandidate(
        const std::vector<uint32_t>& candidate,
        uint32_t bit_length) override;
};
