/***
 * SearchConfig.h – Keresési Konfiguráció
 *
 * Design Pattern: Builder
 * Fluent API a keresési paraméterek beállításához.
 * Támogatja a szekvenciális keresési módot is.
 ***/

#pragma once
#include "observer/ISearchObserver.h"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

class SearchConfig {
public:
    // ── Builder ──
    class Builder {
    public:
        Builder() = default;

        /// Keresett prímek bit mérete (default: 1024)
        Builder& setBitLength(uint32_t bits) {
            config_.bit_length_ = bits;
            return *this;
        }

        /// Hány prímet keressen (default: 0 = végtelen)
        Builder& setPrimeCount(uint32_t count) {
            config_.target_prime_count_ = count;
            return *this;
        }

        /// GPU batch méret – egyszerre ennyi jelöltet tesztel (default: 10000)
        Builder& setBatchSize(uint32_t size) {
            config_.batch_size_ = size;
            return *this;
        }

        /// Miller-Rabin iterációk száma (default: 20)
        Builder& setMillerRabinRounds(uint32_t rounds) {
            config_.mr_rounds_ = rounds;
            return *this;
        }

        /// Stratégia neve: "millerrabin" vagy "trial" (default: "trial")
        Builder& setStrategy(const std::string& name) {
            config_.strategy_name_ = name;
            return *this;
        }

        /// Kimeneti/resume fájl (default: "primes.txt")
        Builder& setOutputFile(const std::string& path) {
            config_.output_file_ = path;
            return *this;
        }

        /// Observer hozzáadása
        Builder& addObserver(std::shared_ptr<ISearchObserver> observer) {
            config_.observers_.push_back(std::move(observer));
            return *this;
        }

        /// Trial division előszűrés engedélyezése (default: true)
        Builder& setPrefilter(bool enabled) {
            config_.use_prefilter_ = enabled;
            return *this;
        }

        /// Verbose konzol kimenet (default: true)
        Builder& setVerbose(bool verbose) {
            config_.verbose_ = verbose;
            return *this;
        }

        /// Szekvenciális mód: 2-től végtelenig (default: true)
        Builder& setSequentialMode(bool enabled) {
            config_.sequential_mode_ = enabled;
            return *this;
        }

        SearchConfig build() { return std::move(config_); }

    private:
        SearchConfig config_;
    };

    // ── Getterek ──
    uint32_t bitLength() const { return bit_length_; }
    uint32_t targetPrimeCount() const { return target_prime_count_; }
    uint32_t batchSize() const { return batch_size_; }
    uint32_t millerRabinRounds() const { return mr_rounds_; }
    const std::string& strategyName() const { return strategy_name_; }
    const std::string& outputFile() const { return output_file_; }
    bool usePrefilter() const { return use_prefilter_; }
    bool verbose() const { return verbose_; }
    bool sequentialMode() const { return sequential_mode_; }
    const std::vector<std::shared_ptr<ISearchObserver>>& observers() const {
        return observers_;
    }

private:
    uint32_t    bit_length_           = 1024;
    uint32_t    target_prime_count_   = 0;       // 0 = végtelen
    uint32_t    batch_size_           = 10000;
    uint32_t    mr_rounds_            = 20;
    std::string strategy_name_        = "trial";  // szekvenciálishoz trial
    std::string output_file_          = "primes.txt";
    bool        use_prefilter_        = true;
    bool        verbose_              = true;
    bool        sequential_mode_      = true;     // alapból szekvenciális
    std::vector<std::shared_ptr<ISearchObserver>> observers_;
};
