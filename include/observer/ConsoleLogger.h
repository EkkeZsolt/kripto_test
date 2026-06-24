/***
 * ConsoleLogger.h – Konzol Observer
 *
 * Design Pattern: Observer (Concrete Observer)
 * Színes konzol kimenet a keresés haladásáról és eredményeiről.
 ***/

#pragma once
#include "observer/ISearchObserver.h"

class ConsoleLogger : public ISearchObserver {
public:
    /// @param verbose Ha true, minden batch után kiír haladást
    explicit ConsoleLogger(bool verbose = true);

    void onPrimeFound(const std::string& prime_hex,
                      uint32_t bit_length,
                      uint64_t prime_index) override;

    void onProgress(uint64_t candidates_tested,
                    uint64_t primes_found,
                    double elapsed_seconds,
                    double candidates_per_sec) override;

    void onSearchComplete(uint64_t total_tested,
                          uint64_t total_found,
                          double total_seconds) override;

private:
    bool verbose_;
};
