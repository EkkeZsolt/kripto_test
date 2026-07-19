/***
 * BigIntConverter.cpp – String ↔ cgbn_mem_t Konverzió Implementáció
 ***/

#include "utils/BigIntConverter.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// ────────────────────────────────────────────────────────
// Decimális string → limb tömb
// Módszer: a decimális stringet 9 jegyenként feldolgozzuk,
// és schoolbook szorzással konvertáljuk binárisba.
// ────────────────────────────────────────────────────────
void BigIntConverter::fromDecimal(const std::string& decimal,
                                  uint32_t* limbs, uint32_t max_limbs) {
    std::memset(limbs, 0, sizeof(uint32_t) * max_limbs);

    if (decimal.empty() || decimal == "0") return;

    // Előjel nélküli decimális string
    std::string dec = decimal;
    if (dec[0] == '+') dec = dec.substr(1);

    for (char c : dec) {
        if (c < '0' || c > '9') {
            throw std::invalid_argument("Invalid decimal digit: " + std::string(1, c));
        }

        // limbs *= 10
        uint64_t carry = 0;
        for (uint32_t i = 0; i < max_limbs; i++) {
            uint64_t val = (uint64_t)limbs[i] * 10 + carry;
            limbs[i] = (uint32_t)(val & 0xFFFFFFFF);
            carry = val >> 32;
        }

        // limbs += digit
        uint64_t add_carry = (uint64_t)(c - '0');
        for (uint32_t i = 0; i < max_limbs && add_carry > 0; i++) {
            uint64_t val = (uint64_t)limbs[i] + add_carry;
            limbs[i] = (uint32_t)(val & 0xFFFFFFFF);
            add_carry = val >> 32;
        }
    }
}

// ────────────────────────────────────────────────────────
// Limb tömb → decimális string
// Módszer: ismételt osztás 10-zel, maradékok gyűjtése
// ────────────────────────────────────────────────────────
std::string BigIntConverter::toDecimal(const uint32_t* limbs, uint32_t num_limbs) {
    // Másolat készítése (destruktív az osztás)
    std::vector<uint32_t> temp(limbs, limbs + num_limbs);

    // Ellenőrzés: nulla-e?
    bool all_zero = true;
    for (uint32_t i = 0; i < num_limbs; i++) {
        if (temp[i] != 0) { all_zero = false; break; }
    }
    if (all_zero) return "0";

    std::string result;
    while (true) {
        // Ellenőrzés: mind nulla?
        all_zero = true;
        for (uint32_t i = 0; i < num_limbs; i++) {
            if (temp[i] != 0) { all_zero = false; break; }
        }
        if (all_zero) break;

        // Osztás 10-zel, maradék gyűjtése
        uint64_t remainder = 0;
        for (int i = (int)num_limbs - 1; i >= 0; i--) {
            uint64_t val = (remainder << 32) | (uint64_t)temp[i];
            temp[i] = (uint32_t)(val / 10);
            remainder = val % 10;
        }
        result += (char)('0' + remainder);
    }

    std::reverse(result.begin(), result.end());
    return result;
}

// ────────────────────────────────────────────────────────
// Hexadecimális string → limb tömb
// ────────────────────────────────────────────────────────
void BigIntConverter::fromHex(const std::string& hex,
                              uint32_t* limbs, uint32_t max_limbs) {
    std::memset(limbs, 0, sizeof(uint32_t) * max_limbs);

    std::string h = hex;
    // "0x" prefix eltávolítása
    if (h.size() >= 2 && h[0] == '0' && (h[1] == 'x' || h[1] == 'X')) {
        h = h.substr(2);
    }
    // Vezető nullák eltávolítása
    while (h.size() > 1 && h[0] == '0') h = h.substr(1);

    // Jobbról balra, 8 hex jegyenként (= 1 uint32_t)
    uint32_t limb_idx = 0;
    int pos = (int)h.size();
    while (pos > 0 && limb_idx < max_limbs) {
        int start = std::max(0, pos - 8);
        std::string chunk = h.substr(start, pos - start);
        limbs[limb_idx] = (uint32_t)std::stoul(chunk, nullptr, 16);
        limb_idx++;
        pos = start;
    }
}

// ────────────────────────────────────────────────────────
// Limb tömb → hexadecimális string
// ────────────────────────────────────────────────────────
std::string BigIntConverter::toHex(const uint32_t* limbs, uint32_t num_limbs) {
    // Legmagasabb nem-nulla limb keresése
    int top = (int)num_limbs - 1;
    while (top > 0 && limbs[top] == 0) top--;

    if (top == 0 && limbs[0] == 0) return "0";

    std::ostringstream oss;
    // Első limb vezető nullák nélkül
    oss << std::hex << limbs[top];
    // Többi limb 8 jegyű padding-gel
    for (int i = top - 1; i >= 0; i--) {
        oss << std::setfill('0') << std::setw(8) << std::hex << limbs[i];
    }
    return oss.str();
}

// ────────────────────────────────────────────────────────
// uint64_t → limb tömb
// ────────────────────────────────────────────────────────
void BigIntConverter::fromUint64(uint64_t value, uint32_t* limbs, uint32_t max_limbs) {
    std::memset(limbs, 0, sizeof(uint32_t) * max_limbs);
    if (max_limbs >= 1) limbs[0] = (uint32_t)(value & 0xFFFFFFFF);
    if (max_limbs >= 2) limbs[1] = (uint32_t)(value >> 32);
}

// ────────────────────────────────────────────────────────
// Random nagy szám generálás
// ────────────────────────────────────────────────────────
void BigIntConverter::randomBits(uint32_t* limbs, uint32_t max_limbs,
                                  uint32_t bit_length, bool make_odd) {
    std::memset(limbs, 0, sizeof(uint32_t) * max_limbs);

    // Random seed a rendszerórából
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(static_cast<uint32_t>(seed));
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

    uint32_t full_limbs = bit_length / 32;
    uint32_t remaining_bits = bit_length % 32;

    // Limb korlát ellenőrzés
    if (full_limbs + (remaining_bits > 0 ? 1 : 0) > max_limbs) {
        throw std::invalid_argument("bit_length too large for max_limbs");
    }

    // Random limb-ek feltöltése
    for (uint32_t i = 0; i < full_limbs && i < max_limbs; i++) {
        limbs[i] = dist(rng);
    }

    // Utolsó (részleges) limb
    if (remaining_bits > 0 && full_limbs < max_limbs) {
        uint32_t mask = (1u << remaining_bits) - 1;
        limbs[full_limbs] = dist(rng) & mask;
        // Legmagasabb bit beállítása (hogy tényleg bit_length bites legyen)
        limbs[full_limbs] |= (1u << (remaining_bits - 1));
    } else if (full_limbs > 0) {
        // Legmagasabb bit beállítása az utolsó teljes limb-ben
        limbs[full_limbs - 1] |= (1u << 31);
    }

    // Páratlanná tétel (prím jelölt)
    if (make_odd) {
        limbs[0] |= 1;
    }
}
