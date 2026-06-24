/***
 * PrimeSearcher.cpp – Keresési Motor Implementáció
 ***/

#include "search/PrimeSearcher.h"
#include "utils/BigIntConverter.h"
#include <chrono>
#include <iostream>

PrimeSearcher::PrimeSearcher(SearchConfig config,
                              std::unique_ptr<IPrimalityStrategy> strategy,
                              std::unique_ptr<IPrimalityStrategy> prefilter)
    : config_(std::move(config))
    , strategy_(std::move(strategy))
    , prefilter_(std::move(prefilter)) {}

std::vector<std::vector<uint32_t>> PrimeSearcher::generateCandidateBatch(uint32_t count) {
    const uint32_t num_limbs = config_.bitLength() / 32;
    std::vector<std::vector<uint32_t>> batch;
    batch.reserve(count);

    for (uint32_t i = 0; i < count; i++) {
        std::vector<uint32_t> limbs(num_limbs, 0);
        BigIntConverter::randomBits(limbs.data(), num_limbs,
                                    config_.bitLength(), true);
        batch.push_back(std::move(limbs));
    }
    return batch;
}

void PrimeSearcher::notifyPrimeFound(const std::string& prime_hex, uint32_t bit_length) {
    for (auto& obs : config_.observers()) {
        obs->onPrimeFound(prime_hex, bit_length, total_found_);
    }
}

void PrimeSearcher::notifyProgress(double elapsed_seconds) {
    double speed = elapsed_seconds > 0 ? total_tested_ / elapsed_seconds : 0;
    for (auto& obs : config_.observers()) {
        obs->onProgress(total_tested_, total_found_, elapsed_seconds, speed);
    }
}

void PrimeSearcher::notifyComplete(double total_seconds) {
    for (auto& obs : config_.observers()) {
        obs->onSearchComplete(total_tested_, total_found_, total_seconds);
    }
}

std::vector<std::string> PrimeSearcher::search() {
    std::vector<std::string> found_primes;
    auto start_time = std::chrono::high_resolution_clock::now();

    std::cout << "\n  Kereses inditas: " << config_.bitLength() << " bites primek"
              << "\n  Strategia: " << strategy_->name()
              << "\n  Batch meret: " << config_.batchSize()
              << "\n  Miller-Rabin korok: " << config_.millerRabinRounds()
              << "\n  Cel: " << config_.targetPrimeCount() << " prim"
              << "\n" << std::endl;

    while (total_found_ < config_.targetPrimeCount()) {
        // 1. Random jelöltek generálása
        auto candidates = generateCandidateBatch(config_.batchSize());

        // 2. Opcionális előszűrés trial division-nel
        if (prefilter_ && config_.usePrefilter()) {
            auto pre_results = prefilter_->testBatch(candidates, config_.bitLength());

            // Csak az előszűrést átment jelölteket tartjuk meg
            std::vector<std::vector<uint32_t>> filtered;
            for (const auto& r : pre_results) {
                if (r.is_probably_prime) {
                    filtered.push_back(std::move(candidates[r.index]));
                }
            }
            candidates = std::move(filtered);
        }

        if (candidates.empty()) {
            total_tested_ += config_.batchSize();
            continue;
        }

        // 3. Miller-Rabin GPU teszt
        auto results = strategy_->testBatch(candidates, config_.bitLength());
        total_tested_ += config_.batchSize();

        // 4. Prímek feldolgozása
        for (const auto& r : results) {
            if (r.is_probably_prime && total_found_ < config_.targetPrimeCount()) {
                total_found_++;
                const auto& limbs = candidates[r.index];
                std::string hex = BigIntConverter::toHex(limbs.data(), (uint32_t)limbs.size());
                found_primes.push_back(hex);
                notifyPrimeFound(hex, config_.bitLength());
            }
        }

        // 5. Haladás jelentés
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();
        notifyProgress(elapsed);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double total_sec = std::chrono::duration<double>(end_time - start_time).count();
    notifyComplete(total_sec);

    return found_primes;
}
