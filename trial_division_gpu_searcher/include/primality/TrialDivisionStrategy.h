#pragma once
#include "primality/IPrimalityStrategy.h"
#include <vector>

class TrialDivisionStrategy : public IPrimalityStrategy {
private:
    std::vector<std::vector<uint32_t>> known_primes;

public:
    TrialDivisionStrategy();
    ~TrialDivisionStrategy() override = default;

    std::string name() const override;
    
    void addKnownPrime(const std::vector<uint32_t>& prime_limbs) override;

    bool testSingleCandidate(
        const std::vector<uint32_t>& candidate,
        uint32_t bit_length) override;
};
