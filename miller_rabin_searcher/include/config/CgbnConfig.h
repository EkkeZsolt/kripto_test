/***
 * CgbnConfig.h – CGBN Template Paraméterek
 *
 * A CGBN cooperative groups megközelítést használ:
 * több GPU thread (TPI) dolgozik együtt egy nagy számon.
 * 4096 bitnél TPI=32 az optimális konfiguráció.
 ***/

#pragma once
#include <cstdint>

/// CGBN paraméter konténer template
/// @tparam tpi         Threads per instance (4, 8, 16, vagy 32)
/// @tparam bits        Instance bit méret (32-tól 32K-ig, 32 többszöröse)
/// @tparam window_bits Windowed exponentiation ablak méret
template<uint32_t tpi, uint32_t bits, uint32_t window_bits>
struct CgbnParams {
    // ── CGBN context paraméterek ──
    static const uint32_t TPB          = 0;      // TPB a blockDim.x-ből jön
    static const uint32_t MAX_ROTATION = 4;      // empirikusan jó alapérték
    static const uint32_t SHM_LIMIT    = 0;      // nincs shared memory limit
    static const bool     CONSTANT_TIME = false; // nem elérhető még a CGBN-ben

    // ── Alkalmazás paraméterek ──
    static const uint32_t TPI         = tpi;
    static const uint32_t BITS        = bits;
    static const uint32_t WINDOW_BITS = window_bits;
};

// ──────────────────────────────────────────────
// Előre definiált konfigurációk
// ──────────────────────────────────────────────

/// 4096 bites konfiguráció – maximális méret a projektben
using Params4096 = CgbnParams<32, 4096, 5>;

/// 2048 bites konfiguráció – gyorsabb, kisebb prímekhez
using Params2048 = CgbnParams<32, 2048, 5>;

/// 1024 bites konfiguráció – leggyorsabb
using Params1024 = CgbnParams<32, 1024, 5>;

/// Alapértelmezett konfiguráció
using DefaultParams = Params4096;
