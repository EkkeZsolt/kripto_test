/***
 * IPrimalityStrategy.h – Strategy Pattern Interface
 *
 * Design Pattern: Strategy
 * Lehetővé teszi különböző prímteszt algoritmusok cserélését
 * futásidőben (Trial Division, Miller-Rabin GPU, stb.)
 ***/

#pragma once
#include <string>
#include <vector>
#include <cstdint>

/// Prímteszt eredmény egyetlen jelöltre
struct PrimalityResult {
    uint32_t index;             ///< Eredeti index a batch-ben
    bool     is_probably_prime; ///< true ha valószínűleg prím
    uint32_t rounds_passed;     ///< Hány Miller-Rabin kört ment át
};

/// Abstract Strategy interface prímtesztekhez
class IPrimalityStrategy {
public:
    virtual ~IPrimalityStrategy() = default;

    /// @return A stratégia neve (pl. "MillerRabin-GPU", "TrialDivision-CPU")
    virtual std::string name() const = 0;

    /// Batch prímteszt: egyszerre több jelöltet tesztel
    /// @param candidates  Jelölt számok limb tömbökként
    ///                    Minden belső vector egy szám limb-jeit tartalmazza
    /// @param bit_length  A számok bit mérete
    /// @return Eredmények minden jelöltre
    virtual std::vector<PrimalityResult> testBatch(
        const std::vector<std::vector<uint32_t>>& candidates,
        uint32_t bit_length) = 0;
};
