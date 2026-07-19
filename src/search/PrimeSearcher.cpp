/***
 * PrimeSearcher.cpp – Keresési Motor Implementáció
 ***/

#include "search/PrimeSearcher.h"
#include "utils/BigIntConverter.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>

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

void PrimeSearcher::notifyPrimeFound(const std::string& prime_dec, uint32_t bit_length) {
    for (auto& obs : config_.observers()) {
        obs->onPrimeFound(prime_dec, bit_length, total_found_);
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

void PrimeSearcher::addUi32(std::vector<uint32_t>& limbs, uint32_t val) {
    uint64_t carry = val;
    for (size_t i = 0; i < limbs.size() && carry > 0; i++) {
        uint64_t sum = (uint64_t)limbs[i] + carry;
        limbs[i] = (uint32_t)(sum & 0xFFFFFFFF);
        carry = sum >> 32;
    }
}

void PrimeSearcher::initializeSequentialState() {
    const uint32_t num_limbs = config_.bitLength() / 32;
    current_sequential_candidate_.assign(num_limbs, 0);
    
    // Alapértelmezett kezdőérték: 3 (az első vizsgálandó páratlan prím jelölt 2 után)
    current_sequential_candidate_[0] = 3;

    bool started_from_config = false;
    if (!config_.startNumber().empty()) {
        BigIntConverter::fromDecimal(config_.startNumber(), current_sequential_candidate_.data(), num_limbs);
        if ((current_sequential_candidate_[0] & 1) == 0) {
            addUi32(current_sequential_candidate_, 1);
        }
        started_from_config = true;
        std::cout << "  Kezdes egyedi szamtol: " << config_.startNumber() << "\n";
    }

    // Csak akkor próbáljuk meg kiolvasni az utolsó prímet a fájlból, ha nem adtunk meg egyedi kezdőértéket
    if (!started_from_config && !config_.outputFile().empty()) {
        std::ifstream file(config_.outputFile());
        if (file.is_open()) {
            std::string line, last_dec;
            uint64_t last_index = 0;
            while (std::getline(file, line)) {
                // Keresünk egy nem üres sort, ami nem komment
                if (!line.empty() && line[0] != '#') {
                    last_index++;
                    last_dec = line;
                }
            }
            if (!last_dec.empty()) {
                total_found_ = last_index;
                BigIntConverter::fromDecimal(last_dec, current_sequential_candidate_.data(), num_limbs);
                
                // Biztosítjuk, hogy a következő jelölt páratlan legyen
                if (current_sequential_candidate_[0] == 2) {
                    current_sequential_candidate_[0] = 3;
                }
                else if ((current_sequential_candidate_[0] & 1) != 0) {
                    addUi32(current_sequential_candidate_, 2);
                }
                else {
                    addUi32(current_sequential_candidate_, 1);
                }
                
                std::cout << "  Folytatas innen: " << last_dec << " (Mentett prím index: " << last_index << ")\n";
                return;
            }
            else {
                // Ha nem találtunk mentett prímet, akkor a 2 a legelső prímünk!
                total_found_++;
                notifyPrimeFound("2", config_.bitLength());
            }
        }
    }
    else if (!started_from_config) {
        // Ha nincs kimeneti fájl megadva, és nem egyedi kezdőérték van, akkor is a 2 az első
        total_found_++;
        notifyPrimeFound("2", config_.bitLength());
    }
}

std::vector<std::vector<uint32_t>> PrimeSearcher::generateSequentialBatch(uint32_t count) {
    std::vector<std::vector<uint32_t>> batch;
    batch.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        batch.push_back(current_sequential_candidate_);
        // Következő páratlan szám (+2)
        addUi32(current_sequential_candidate_, 2);
    }
    return batch;
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

    if (config_.sequentialMode()) {
        initializeSequentialState();
    }

    while (config_.targetPrimeCount() == 0 || total_found_ < config_.targetPrimeCount()) {
        // 1. Jelöltek generálása (random vagy szekvenciális)
        auto candidates = config_.sequentialMode() 
            ? generateSequentialBatch(config_.batchSize()) 
            : generateCandidateBatch(config_.batchSize());

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
            if (r.is_probably_prime && (config_.targetPrimeCount() == 0 || total_found_ < config_.targetPrimeCount())) {
                total_found_++;
                const auto& limbs = candidates[r.index];
                std::string dec = BigIntConverter::toDecimal(limbs.data(), (uint32_t)limbs.size());
                found_primes.push_back(dec);
                notifyPrimeFound(dec, config_.bitLength());
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
