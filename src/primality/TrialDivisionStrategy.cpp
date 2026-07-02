/***
 * TrialDivisionStrategy.cpp – CPU Trial Division Implementáció
 ***/

#include "primality/TrialDivisionStrategy.h"
#include <cmath>

TrialDivisionStrategy::TrialDivisionStrategy(uint32_t max_divisor) {
    generateSmallPrimes(max_divisor);
}

std::string TrialDivisionStrategy::name() const {
    return "TrialDivision-CPU";
}

// ────────────────────────────────────────────────────────
// Eratosthenes szita – kis prímek generálása
// ────────────────────────────────────────────────────────
void TrialDivisionStrategy::generateSmallPrimes(uint32_t limit) {
    std::vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;

    for (uint32_t i = 2; i * i <= limit; i++) {
        if (is_prime[i]) {
            for (uint32_t j = i * i; j <= limit; j += i) {
                is_prime[j] = false;
            }
        }
    }

    small_primes_.clear();
    for (uint32_t i = 2; i <= limit; i++) {
        if (is_prime[i]) {
            small_primes_.push_back(i);
        }
    }
}

// ────────────────────────────────────────────────────────
// Oszthatósági teszt: limb tömb % divisor == 0?
// BigInt osztás egy uint32_t-vel: schoolbook maradék
// ────────────────────────────────────────────────────────
bool TrialDivisionStrategy::isDivisible(const uint32_t* limbs,
                                         uint32_t num_limbs,
                                         uint32_t divisor) const {
    uint64_t remainder = 0;
    for (int i = (int)num_limbs - 1; i >= 0; i--) {
        remainder = (remainder << 32) | (uint64_t)limbs[i];
        remainder = remainder % divisor;
    }
    return remainder == 0;
}

static uint64_t getVal64(const uint32_t* limbs, uint32_t num_limbs) {
    if (num_limbs == 0) return 0;
    if (num_limbs == 1) return limbs[0];
    for (uint32_t i = 2; i < num_limbs; i++) {
        if (limbs[i] != 0) return 0xFFFFFFFFFFFFFFFFULL;
    }
    return ((uint64_t)limbs[1] << 32) | limbs[0];
}

// ────────────────────────────────────────────────────────
// Batch teszt: minden jelöltet végigpróbálunk a kis prímekkel
// ────────────────────────────────────────────────────────
std::vector<PrimalityResult> TrialDivisionStrategy::testBatch(
        const std::vector<std::vector<uint32_t>>& candidates,
        uint32_t bit_length) {

    std::vector<PrimalityResult> results;
    results.reserve(candidates.size());
    for (uint32_t idx = 0; idx < candidates.size(); idx++) {
        const auto& limbs = candidates[idx];
        bool composite = false;
        uint64_t val64 = getVal64(limbs.data(), (uint32_t)limbs.size());

        for (uint32_t prime : small_primes_) {
            // Négyzetgyök (sqrt) optimalizáció: ha a vizsgált osztó négyzete nagyobb,
            // mint a jelölt értéke, akkor felesleges tovább keresni, biztosan prím.
            if ((uint64_t)prime * prime > val64) {
                break;
            }

            // Ha a jelölt maga a kis prím, az prím
            bool is_equal = (limbs[0] == prime);
            if (is_equal) {
                for (size_t i = 1; i < limbs.size(); i++) {
                    if (limbs[i] != 0) {
                        is_equal = false;
                        break;
                    }
                }
            }
            if (is_equal) {
                break;
            }
            if (isDivisible(limbs.data(), (uint32_t)limbs.size(), prime)) {
                composite = true;
                break;
            }
        }

        PrimalityResult r;
        r.index = idx;
        r.is_probably_prime = !composite;
        r.rounds_passed = composite ? 0 : (uint32_t)small_primes_.size();
        results.push_back(r);
    }

    return results;
}
