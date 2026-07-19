#include "search/PrimeSearchFactory.h"
#include "primality/GpuTrialDivisionStrategy.h"
#include <stdexcept>

std::unique_ptr<PrimeSearcher> PrimeSearchFactory::create(const SearchConfig& config) {
    // Fő stratégia: GpuTrialDivisionStrategy
    auto strategy = std::make_unique<GpuTrialDivisionStrategy>();

    return std::make_unique<PrimeSearcher>(
        config,
        std::move(strategy)
    );
}
