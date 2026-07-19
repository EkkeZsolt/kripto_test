/***
 * ResultFileWriter.h – Fájl Observer
 *
 * Design Pattern: Observer (Concrete Observer)
 * A talált prímeket fájlba menti.
 ***/

#pragma once
#include "observer/ISearchObserver.h"
#include <fstream>
#include <string>

class ResultFileWriter : public ISearchObserver {
public:
    /// @param filepath Kimeneti fájl útvonala
    explicit ResultFileWriter(const std::string& filepath);
    ~ResultFileWriter() override;

    void onPrimeFound(const std::string& prime_dec,
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
    std::ofstream file_;
    std::string   filepath_;
};
