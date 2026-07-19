/***
 * PrimeSearcher.h – Prímszám Keresési Motor
 *
 * A fő keresési logikát tartalmazza:
 * 1. Random jelöltek generálása (OpenMP párhuzamos)
 * 2. Opcionális trial division előszűrés (OpenMP párhuzamos)
 * 3. Miller-Rabin GPU batch teszt (CUDA stream pipeline)
 * 4. Observer-ek értesítése
 *
 * RTX 3090 + Ryzen 9 5950X optimalizált:
 * - CPU/GPU átfedés: a CPU generálja a következő batch-et, miközben a GPU dolgozik
 * - OpenMP: jelölt generálás és előszűrés mind a 32 szálon
 ***/

#pragma once
#include "search/SearchConfig.h"
#include "primality/IPrimalityStrategy.h"
#include <memory>
#include <vector>
#include <future>

class PrimeSearcher {
public:
    /// @param config    Keresési konfiguráció (Builder-ből)
    /// @param strategy  Fő prímteszt stratégia
    /// @param prefilter Opcionális előszűrő stratégia (trial division)
    PrimeSearcher(SearchConfig config,
                  std::unique_ptr<IPrimalityStrategy> strategy,
                  std::unique_ptr<IPrimalityStrategy> prefilter = nullptr);

    /// Elindítja a keresést a konfigurált paraméterekkel
    /// @return A talált prímek hexadecimális formában
    std::vector<std::string> search();

private:
    SearchConfig                        config_;
    std::unique_ptr<IPrimalityStrategy> strategy_;
    std::unique_ptr<IPrimalityStrategy> prefilter_;
    uint64_t                            total_tested_ = 0;
    uint64_t                            total_found_  = 0;

    /// Következő batch generálása szekvenciálisan
    std::vector<std::vector<uint32_t>> generateCandidateBatch(uint32_t count);

    /// Előszűr és visszaadja a szűrt jelölteket
    std::vector<std::vector<uint32_t>> prefilterCandidates(
        std::vector<std::vector<uint32_t>>& candidates);

    /// Értesíti az összes observer-t prím találatról
    void notifyPrimeFound(const std::string& prime_dec, uint32_t bit_length);

    /// Értesíti az összes observer-t haladásról
    void notifyProgress(double elapsed_seconds);

    /// Értesíti az összes observer-t befejezésről
    void notifyComplete(double total_seconds);

    // Szekvenciális mód állapota
    std::vector<uint32_t> current_sequential_candidate_;

    /// Beállítja a kezdőértéket a szekvenciális kereséshez
    void initializeSequentialState();

    /// BigInt hozzáadása (a += b)
    static void addUi32(std::vector<uint32_t>& limbs, uint32_t val);
};
