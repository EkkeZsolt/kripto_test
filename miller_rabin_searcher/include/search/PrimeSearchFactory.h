/***
 * PrimeSearchFactory.h – Keresési Komponens Factory
 *
 * Design Pattern: Factory
 * SearchConfig alapján összeszereli a teljes keresési rendszert:
 * stratégia + előszűrő + observerek + PrimeSearcher.
 ***/

#pragma once
#include "search/PrimeSearcher.h"
#include "search/SearchConfig.h"
#include <memory>

class PrimeSearchFactory {
public:
    /// Létrehozza a teljes PrimeSearcher-t a konfiguráció alapján
    /// @param config A Builder-ből kapott keresési konfiguráció
    /// @return Teljesen összeszerelt PrimeSearcher
    static std::unique_ptr<PrimeSearcher> create(const SearchConfig& config);
};
