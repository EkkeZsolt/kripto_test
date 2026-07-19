#include "search/PrimeSearcher.h"
#include "utils/BigIntConverter.h"
#include <iostream>
#include <chrono>

PrimeSearcher::PrimeSearcher(SearchConfig config,
                             std::unique_ptr<IPrimalityStrategy> strategy)
    : config_(std::move(config)),
      strategy_(std::move(strategy))
{
    initializeSequentialState();
}

void PrimeSearcher::initializeSequentialState() {
    uint32_t limbs_count = config_.bitLength() / 32;
    current_sequential_candidate_.resize(limbs_count, 0);

    if (config_.startNumber().empty()) {
        // Alapertelmezett kis prim jelolt kezdetnek
        BigIntConverter::fromDecimal("3", current_sequential_candidate_.data(), limbs_count);
    } else {
        BigIntConverter::fromDecimal(config_.startNumber(), current_sequential_candidate_.data(), limbs_count);
    }

    // Make it odd
    if ((current_sequential_candidate_[0] & 1) == 0) {
        addUi32(current_sequential_candidate_, 1);
    }
}

void PrimeSearcher::addUi32(std::vector<uint32_t>& limbs, uint32_t val) {
    uint64_t carry = val;
    for (size_t i = 0; i < limbs.size() && carry > 0; ++i) {
        uint64_t sum = static_cast<uint64_t>(limbs[i]) + carry;
        limbs[i] = static_cast<uint32_t>(sum);
        carry = sum >> 32;
    }
}

std::vector<std::string> PrimeSearcher::search() {
    std::vector<std::string> found_primes;
    auto start_time = std::chrono::steady_clock::now();
    auto last_log = start_time;

    uint32_t limit = config_.targetPrimeCount();
    if (limit == 0) limit = 0xFFFFFFFF;

    while (found_primes.size() < limit) {
        total_tested_++;

        bool is_prime = strategy_->testSingleCandidate(current_sequential_candidate_, config_.bitLength());

        if (is_prime) {
            std::string hex_str = BigIntConverter::toHex(current_sequential_candidate_.data(), config_.bitLength() / 32);
            found_primes.push_back(hex_str);
            total_found_++;
            notifyPrimeFound(hex_str, config_.bitLength());
            
            strategy_->addKnownPrime(current_sequential_candidate_);
        }

        addUi32(current_sequential_candidate_, 2);

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - start_time;
        std::chrono::duration<double> since_log = now - last_log;

        if (since_log.count() >= 1.0) {
            notifyProgress(elapsed.count());
            last_log = now;
        }
    }

    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now - start_time;
    notifyComplete(elapsed.count());

    return found_primes;
}

void PrimeSearcher::notifyPrimeFound(const std::string& prime_hex, uint32_t bit_length) {
    std::cout << "[PrimeSearcher] TALÁLAT! " << bit_length << "-bit: " << prime_hex << std::endl;
}

void PrimeSearcher::notifyProgress(double elapsed_seconds) {
    double speed = static_cast<double>(total_tested_) / elapsed_seconds;
    std::cout << "[PrimeSearcher] Haladas: Tesztelve: " << total_tested_ 
              << ", Talalt: " << total_found_ 
              << ", Sebesseg: " << speed << " jelolt/mp\r" << std::flush;
}

void PrimeSearcher::notifyComplete(double total_seconds) {
    std::cout << "\n[PrimeSearcher] Kereses befejezodott " << total_seconds << " mp alatt." << std::endl;
}
