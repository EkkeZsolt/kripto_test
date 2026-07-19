#pragma once
#include "search/SearchConfig.h"
#include "primality/IPrimalityStrategy.h"
#include <memory>
#include <vector>

class PrimeSearcher {
public:
    PrimeSearcher(SearchConfig config,
                  std::unique_ptr<IPrimalityStrategy> strategy);

    std::vector<std::string> search();

private:
    SearchConfig                        config_;
    std::unique_ptr<IPrimalityStrategy> strategy_;
    uint64_t                            total_tested_ = 0;
    uint64_t                            total_found_  = 0;

    void notifyPrimeFound(const std::string& prime_dec, uint32_t bit_length);
    void notifyProgress(double elapsed_seconds);
    void notifyComplete(double total_seconds);

    std::vector<uint32_t> current_sequential_candidate_;
    void initializeSequentialState();
    static void addUi32(std::vector<uint32_t>& limbs, uint32_t val);
};
