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

    /// @return A stratégia neve (pl. "TrialDivision-GPU")
    virtual std::string name() const = 0;

    /// Bővíti a memóriában (VRAM) tárolt prímek listáját egy újabbal.
    /// Ezt a CPU hívja meg, amikor talál egy új prímet.
    virtual void addKnownPrime(const std::vector<uint32_t>& prime_limbs) = 0;

    /// Tesztel egyetlen jelöltet az összes eddig ismert prímmel.
    /// @param candidate A vizsgálandó szám
    /// @param bit_length A szám bit mérete
    /// @return true, ha a szám nem osztható egyetlen eddigi prímmel sem
    virtual bool testSingleCandidate(
        const std::vector<uint32_t>& candidate,
        uint32_t bit_length) = 0;
};
