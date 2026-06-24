/***
 * test_bigint_converter.cpp – BigIntConverter Unit Tesztek
 ***/

#include <gtest/gtest.h>
#include "utils/BigIntConverter.h"
#include <cstring>

class BigIntConverterTest : public ::testing::Test {
protected:
    static const uint32_t MAX_LIMBS = 128;  // 4096 / 32
    uint32_t limbs[MAX_LIMBS];

    void SetUp() override {
        std::memset(limbs, 0, sizeof(limbs));
    }
};

// ── Decimal konverzió tesztek ──

TEST_F(BigIntConverterTest, ZeroFromDecimal) {
    BigIntConverter::fromDecimal("0", limbs, MAX_LIMBS);
    EXPECT_EQ(limbs[0], 0u);
}

TEST_F(BigIntConverterTest, SmallNumberFromDecimal) {
    BigIntConverter::fromDecimal("42", limbs, MAX_LIMBS);
    EXPECT_EQ(limbs[0], 42u);
}

TEST_F(BigIntConverterTest, LargeNumberFromDecimal) {
    // 2^32 = 4294967296
    BigIntConverter::fromDecimal("4294967296", limbs, MAX_LIMBS);
    EXPECT_EQ(limbs[0], 0u);
    EXPECT_EQ(limbs[1], 1u);
}

TEST_F(BigIntConverterTest, RoundTripDecimal) {
    std::string original = "123456789012345678901234567890";
    BigIntConverter::fromDecimal(original, limbs, MAX_LIMBS);
    std::string result = BigIntConverter::toDecimal(limbs, MAX_LIMBS);
    EXPECT_EQ(result, original);
}

TEST_F(BigIntConverterTest, ToDecimalZero) {
    std::string result = BigIntConverter::toDecimal(limbs, MAX_LIMBS);
    EXPECT_EQ(result, "0");
}

// ── Hex konverzió tesztek ──

TEST_F(BigIntConverterTest, HexFromSimple) {
    BigIntConverter::fromHex("FF", limbs, MAX_LIMBS);
    EXPECT_EQ(limbs[0], 0xFFu);
}

TEST_F(BigIntConverterTest, HexRoundTrip) {
    BigIntConverter::fromHex("DEADBEEF12345678", limbs, MAX_LIMBS);
    std::string hex = BigIntConverter::toHex(limbs, MAX_LIMBS);
    EXPECT_EQ(hex, "deadbeef12345678");
}

TEST_F(BigIntConverterTest, HexWithPrefix) {
    BigIntConverter::fromHex("0xABCD", limbs, MAX_LIMBS);
    EXPECT_EQ(limbs[0], 0xABCDu);
}

// ── Uint64 konverzió ──

TEST_F(BigIntConverterTest, Uint64Simple) {
    BigIntConverter::fromUint64(0xDEADBEEF, limbs, MAX_LIMBS);
    EXPECT_EQ(limbs[0], 0xDEADBEEF);
    EXPECT_EQ(limbs[1], 0u);
}

TEST_F(BigIntConverterTest, Uint64Large) {
    BigIntConverter::fromUint64(0x123456789ABCDEF0ULL, limbs, MAX_LIMBS);
    EXPECT_EQ(limbs[0], 0x9ABCDEF0u);
    EXPECT_EQ(limbs[1], 0x12345678u);
}

// ── Random generálás ──

TEST_F(BigIntConverterTest, RandomBitsOdd) {
    BigIntConverter::randomBits(limbs, MAX_LIMBS, 256, true);
    EXPECT_TRUE(limbs[0] & 1);  // Páratlan
}

TEST_F(BigIntConverterTest, RandomBitsHighBitSet) {
    BigIntConverter::randomBits(limbs, MAX_LIMBS, 128, true);
    // A 128. bit a 4. limb-ben van (index 3), bit 31
    EXPECT_TRUE(limbs[3] & (1u << 31));
}
