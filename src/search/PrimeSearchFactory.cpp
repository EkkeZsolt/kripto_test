/***
 * PrimeSearchFactory.cpp – Factory Implementáció
 *
 * Design Pattern: Factory
 * A konfiguráció alapján összeszereli a teljes keresési rendszert.
 ***/

#include "search/PrimeSearchFactory.h"
#include "primality/TrialDivisionStrategy.h"
#include "primality/MillerRabinGpuStrategy.h"
#include "observer/ConsoleLogger.h"
#include "observer/ResultFileWriter.h"
#include <stdexcept>

std::unique_ptr<PrimeSearcher> PrimeSearchFactory::create(const SearchConfig& config) {
    // ── Fő stratégia létrehozása ──
    std::unique_ptr<IPrimalityStrategy> strategy;

    if (config.strategyName() == "millerrabin" || config.strategyName() == "mr") {
        strategy = std::make_unique<MillerRabinGpuStrategy>(
            config.millerRabinRounds(), 128);
    }
    else if (config.strategyName() == "trial") {
        strategy = std::make_unique<TrialDivisionStrategy>(100000);
    }
    else {
        throw std::invalid_argument(
            "Ismeretlen strategia: " + config.strategyName() +
            " (hasznalj: 'millerrabin' vagy 'trial')");
    }

    // ── Előszűrő (trial division, opcionális) ──
    std::unique_ptr<IPrimalityStrategy> prefilter;
    if (config.usePrefilter() && config.strategyName() != "trial") {
        prefilter = std::make_unique<TrialDivisionStrategy>(10000);
    }

    // ── Observer-ek hozzáadása a config-hoz ──
    // A SearchConfig már tartalmazza a felhasználó által hozzáadott observereket.
    // Itt csak az alapértelmezetteket adjuk hozzá, ha szükséges.
    SearchConfig mutable_config = config;

    // Konzol logger mindig aktív
    if (config.observers().empty()) {
        auto console = std::make_shared<ConsoleLogger>(config.verbose());
        SearchConfig::Builder builder;
        builder.setBitLength(config.bitLength())
               .setPrimeCount(config.targetPrimeCount())
               .setBatchSize(config.batchSize())
               .setMillerRabinRounds(config.millerRabinRounds())
               .setStrategy(config.strategyName())
               .setPrefilter(config.usePrefilter())
               .setVerbose(config.verbose())
               .setSequentialMode(config.sequentialMode())
               .setStartNumber(config.startNumber())
               .addObserver(console);

        if (!config.outputFile().empty()) {
            auto file_writer = std::make_shared<ResultFileWriter>(config.outputFile());
            builder.addObserver(file_writer);
            builder.setOutputFile(config.outputFile());
        }

        mutable_config = builder.build();
    }

    // ── PrimeSearcher összeszerelése ──
    return std::make_unique<PrimeSearcher>(
        std::move(mutable_config),
        std::move(strategy),
        std::move(prefilter)
    );
}
