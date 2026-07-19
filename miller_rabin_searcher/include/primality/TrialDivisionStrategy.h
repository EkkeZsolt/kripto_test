/***
 * TrialDivisionStrategy.h – CPU Trial Division
 *
 * Design Pattern: Strategy (Concrete Strategy)
 * Egyszerű próbaosztás kis prímekkel.
 * Használható előszűrésre vagy kis számok tesztelésére.
 ***/

#pragma once
#include "primality/IPrimalityStrategy.h"

class TrialDivisionStrategy : public IPrimalityStrategy {
public:
    /// @param max_divisor Maximális próbaosztó (default: 10000)
    explicit TrialDivisionStrategy(uint32_t max_divisor = 10000);

    std::string name() const override;

    std::vector<PrimalityResult> testBatch(
        const std::vector<std::vector<uint32_t>>& candidates,
        uint32_t bit_length) override;

private:
    /// Kis prímek listája az előszűréshez
    std::vector<uint32_t> small_primes_;

    /// Generálja a kis prímek listáját Eratosthenes szitával
    void generateSmallPrimes(uint32_t limit);

    /// Ellenőrzi, hogy a szám osztható-e a megadott kis prímmel
    /// @return true ha osztható (tehát NEM prím)
    bool isDivisible(const uint32_t* limbs, uint32_t num_limbs, uint32_t divisor) const;
};
