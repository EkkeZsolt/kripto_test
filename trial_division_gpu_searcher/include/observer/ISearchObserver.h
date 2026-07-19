/***
 * ISearchObserver.h – Observer Pattern Interface
 *
 * Design Pattern: Observer
 * Lehetővé teszi a keresési eseményekre való feliratkozást
 * anélkül, hogy a PrimeSearcher tudná, ki figyeli.
 ***/

#pragma once
#include <string>
#include <cstdint>

/// Abstract Observer interface keresési eseményekhez
class ISearchObserver {
public:
    virtual ~ISearchObserver() = default;

    /// Új prím találat
    /// @param prime_dec    A talált prím decimális formában
    /// @param bit_length   A prím bit hossza
    /// @param prime_index  Hányadik talált prím (1-től)
    virtual void onPrimeFound(const std::string& prime_dec,
                              uint32_t bit_length,
                              uint64_t prime_index) = 0;

    /// Haladás jelentés
    /// @param candidates_tested  Eddig tesztelt jelöltek száma
    /// @param primes_found       Eddig talált prímek száma
    /// @param elapsed_seconds    Eltelt idő másodpercben
    /// @param candidates_per_sec Jelöltek/másodperc sebesség
    virtual void onProgress(uint64_t candidates_tested,
                            uint64_t primes_found,
                            double elapsed_seconds,
                            double candidates_per_sec) = 0;

    /// Keresés befejezése
    /// @param total_tested  Összes tesztelt jelölt
    /// @param total_found   Összes talált prím
    /// @param total_seconds Összes eltelt idő
    virtual void onSearchComplete(uint64_t total_tested,
                                  uint64_t total_found,
                                  double total_seconds) = 0;
};
