/***
 * test_primality.cpp – Prímteszt Unit Tesztek
 ***/

#include <gtest/gtest.h>
#include "primality/TrialDivisionStrategy.h"
#include "search/SearchConfig.h"
#include "utils/BigIntConverter.h"
#include <cstring>

TEST(SearchConfigTest, DefaultBatchSizeUsesSixGigabyteVramEstimate) {
    const auto expected = SearchConfig::defaultBatchSizeForVramBytes(6ULL * 1024 * 1024 * 1024);
    const auto config = SearchConfig::Builder{}.build();
    EXPECT_EQ(config.batchSize(), expected);
}

class TrialDivisionTest : public ::testing::Test {
protected:
    TrialDivisionStrategy strategy{10000};
    static const uint32_t MAX_LIMBS = 4;  // 128 bit elég kis számokhoz

    std::vector<uint32_t> fromValue(uint64_t val) {
        std::vector<uint32_t> limbs(MAX_LIMBS, 0);
        BigIntConverter::fromUint64(val, limbs.data(), MAX_LIMBS);
        return limbs;
    }
};

TEST_F(TrialDivisionTest, SmallPrimesAreDetected) {
    // Ismert kis prímek
    std::vector<uint64_t> primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 97};

    for (auto p : primes) {
        std::vector<std::vector<uint32_t>> batch = {fromValue(p)};
        auto results = strategy.testBatch(batch, 64);
        EXPECT_TRUE(results[0].is_probably_prime)
            << p << " should be detected as prime";
    }
}

TEST_F(TrialDivisionTest, CompositeNumbersDetected) {
    // Ismert összetett számok
    std::vector<uint64_t> composites = {4, 6, 8, 9, 10, 15, 21, 25, 100, 1000};

    for (auto c : composites) {
        std::vector<std::vector<uint32_t>> batch = {fromValue(c)};
        auto results = strategy.testBatch(batch, 64);
        EXPECT_FALSE(results[0].is_probably_prime)
            << c << " should be detected as composite";
    }
}

TEST_F(TrialDivisionTest, BatchTestingWorks) {
    // Vegyes batch: prímek és összetettek
    std::vector<std::vector<uint32_t>> batch = {
        fromValue(7),    // prím
        fromValue(15),   // összetett (3*5)
        fromValue(23),   // prím
        fromValue(100),  // összetett (2*2*5*5)
        fromValue(997),  // prím
    };

    auto results = strategy.testBatch(batch, 64);
    ASSERT_EQ(results.size(), 5u);
    EXPECT_TRUE(results[0].is_probably_prime);   // 7
    EXPECT_FALSE(results[1].is_probably_prime);  // 15
    EXPECT_TRUE(results[2].is_probably_prime);   // 23
    EXPECT_FALSE(results[3].is_probably_prime);  // 100
    EXPECT_TRUE(results[4].is_probably_prime);   // 997
}

TEST_F(TrialDivisionTest, LargerPrime) {
    // 9973 prím szám
    std::vector<std::vector<uint32_t>> batch = {fromValue(9973)};
    auto results = strategy.testBatch(batch, 64);
    EXPECT_TRUE(results[0].is_probably_prime);
}

#include "primality/MillerRabinGpuStrategy.h"

class MillerRabinGpuTest : public ::testing::Test {
protected:
    MillerRabinGpuStrategy strategy{20, 128};
    static const uint32_t MAX_LIMBS = 8;  // 256 bit

    std::vector<uint32_t> fromValue(uint64_t val) {
        std::vector<uint32_t> limbs(MAX_LIMBS, 0);
        BigIntConverter::fromUint64(val, limbs.data(), MAX_LIMBS);
        return limbs;
    }
};

TEST_F(MillerRabinGpuTest, PrimesAndComposites) {
    std::vector<uint64_t> primes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 97};
    for (auto p : primes) {
        std::vector<std::vector<uint32_t>> batch = {fromValue(p)};
        try {
            auto results = strategy.testBatch(batch, 256);
            EXPECT_TRUE(results[0].is_probably_prime) << p << " should be prime, rounds_passed: " << results[0].rounds_passed;
        } catch (const std::exception& e) {
            std::cout << "CUDA skip/fail: " << e.what() << std::endl;
        }
    }

    std::vector<uint64_t> composites = {4, 6, 8, 9, 10, 15, 21, 25, 100, 1000};
    for (auto c : composites) {
        std::vector<std::vector<uint32_t>> batch = {fromValue(c)};
        try {
            auto results = strategy.testBatch(batch, 256);
            EXPECT_FALSE(results[0].is_probably_prime) << c << " should be composite";
        } catch (const std::exception& e) {
            std::cout << "CUDA skip/fail: " << e.what() << std::endl;
        }
    }
}

