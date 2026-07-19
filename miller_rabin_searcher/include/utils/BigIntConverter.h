/***
 * BigIntConverter.h – String ↔ cgbn_mem_t Konverzió
 *
 * Decimális string reprezentációt alakít CGBN memória formátumra
 * és vissza. GMP nélkül, saját implementáció.
 ***/

#pragma once
#include <string>
#include <vector>
#include <cstdint>
class BigIntConverter {
public:
    /// Decimális string → uint32_t limb tömb (little-endian)
    /// @param decimal  Decimális szám stringként (pl. "12345678901234567890")
    /// @param limbs    Kimeneti limb tömb
    /// @param max_limbs Maximum limb szám (pl. 128 = 4096/32)
    static void fromDecimal(const std::string& decimal,
                            uint32_t* limbs, uint32_t max_limbs);

    /// uint32_t limb tömb → decimális string
    /// @param limbs    Limb tömb (little-endian)
    /// @param num_limbs Limb-ek száma
    /// @return Decimális string reprezentáció
    static std::string toDecimal(const uint32_t* limbs, uint32_t num_limbs);

    /// Hexadecimális string → limb tömb
    static void fromHex(const std::string& hex,
                        uint32_t* limbs, uint32_t max_limbs);

    /// Limb tömb → hexadecimális string
    static std::string toHex(const uint32_t* limbs, uint32_t num_limbs);

    /// uint64_t → limb tömb
    static void fromUint64(uint64_t value, uint32_t* limbs, uint32_t max_limbs);

    /// Random nagy szám generálása adott bit mérettel
    /// @param limbs      Kimeneti limb tömb
    /// @param max_limbs  Maximum limb szám
    /// @param bit_length Kívánt bit hossz
    /// @param make_odd   Ha true, páratlanra állítja (prím jelöltnek)
    static void randomBits(uint32_t* limbs, uint32_t max_limbs,
                           uint32_t bit_length, bool make_odd = true);
};
