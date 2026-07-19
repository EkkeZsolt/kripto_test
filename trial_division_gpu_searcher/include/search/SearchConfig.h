/***
 * SearchConfig.h – Keresési Konfiguráció
 *
 * Design Pattern: Builder
 * Fluent API a keresési paraméterek beállításához.
 * Támogatja a szekvenciális keresési módot is.
 ***/

#pragma once
#include "observer/ISearchObserver.h"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <vector>

class SearchConfig {
public:
    // RTX 3090: 24 GB VRAM – 20 GiB használunk, tartalék a CUDA overheadre
    static constexpr uint64_t kDefaultVramBytes = 20ULL * 1024 * 1024 * 1024;
    static constexpr uint32_t kMinimumBatchSizeForVram = 0;

    static uint32_t defaultBatchSizeForVramBytes(uint64_t vram_bytes) {
        constexpr uint32_t bytes_per_candidate = 516; // 4096-bit CGBN instance + passed flag
        if (vram_bytes <= bytes_per_candidate) {
            return 1;
        }

        uint64_t batch_size = vram_bytes / bytes_per_candidate;
        batch_size = std::min<uint64_t>(batch_size, std::numeric_limits<uint32_t>::max());
        return static_cast<uint32_t>(batch_size);
    }

    // ── Builder ──
    class Builder {
    public:
        Builder() = default;

        Builder& setBitLength(uint32_t bits) {
            bit_length_ = bits;
            return *this;
        }
        Builder& setPrimeCount(uint32_t count) {
            target_prime_count_ = count;
            return *this;
        }
        Builder& setBatchSize(uint32_t size) {
            batch_size_ = std::max(size, SearchConfig::defaultBatchSizeForVramBytes(SearchConfig::kDefaultVramBytes));
            return *this;
        }
        Builder& setMillerRabinRounds(uint32_t rounds) {
            mr_rounds_ = rounds;
            return *this;
        }
        Builder& setStrategy(const std::string& name) {
            strategy_name_ = name;
            return *this;
        }
        Builder& setOutputFile(const std::string& path) {
            output_file_ = path;
            return *this;
        }
        Builder& addObserver(std::shared_ptr<ISearchObserver> observer) {
            observers_.push_back(std::move(observer));
            return *this;
        }
        Builder& setPrefilter(bool enabled) {
            use_prefilter_ = enabled;
            return *this;
        }
        Builder& setVerbose(bool verbose) {
            verbose_ = verbose;
            return *this;
        }
        Builder& setStartNumber(const std::string& start_dec) {
            start_number_ = start_dec;
            return *this;
        }

        SearchConfig build() {
            SearchConfig config;
            config.bit_length_ = bit_length_;
            config.target_prime_count_ = target_prime_count_;
            config.batch_size_ = batch_size_;
            config.mr_rounds_ = mr_rounds_;
            config.strategy_name_ = strategy_name_;
            config.output_file_ = output_file_;
            config.use_prefilter_ = use_prefilter_;
            config.verbose_ = verbose_;
            config.start_number_ = start_number_;
            config.observers_ = std::move(observers_);
            return config;
        }

    private:
        uint32_t    bit_length_           = 256;
        uint32_t    target_prime_count_   = 0;
        uint32_t    batch_size_           = SearchConfig::defaultBatchSizeForVramBytes(SearchConfig::kDefaultVramBytes);
        uint32_t    mr_rounds_            = 20;
        std::string strategy_name_        = "mr";
        std::string output_file_          = "primes.txt";
        bool        use_prefilter_        = true;
        bool        verbose_              = true;
        std::string start_number_         = "";
        std::vector<std::shared_ptr<ISearchObserver>> observers_;
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
    const std::string& startNumber() const { return start_number_; }
    const std::vector<std::shared_ptr<ISearchObserver>>& observers() const {
        return observers_;
    }

private:
    uint32_t    bit_length_           = 256;
    uint32_t    target_prime_count_   = 0;       // 0 = végtelen
    uint32_t    batch_size_           = SearchConfig::defaultBatchSizeForVramBytes(SearchConfig::kDefaultVramBytes);
    uint32_t    mr_rounds_            = 20;
    std::string strategy_name_        = "mr";  // szekvenciálishoz alapból mr
    std::string output_file_          = "primes.txt";
    bool        use_prefilter_        = true;
    bool        verbose_              = true;
    std::string start_number_         = "";
    std::vector<std::shared_ptr<ISearchObserver>> observers_;
};
