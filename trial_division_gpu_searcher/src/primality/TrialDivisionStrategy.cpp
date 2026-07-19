#include "primality/TrialDivisionStrategy.h"
#include <iostream>

TrialDivisionStrategy::TrialDivisionStrategy() {
    // Default constructor
}

std::string TrialDivisionStrategy::name() const {
    return "TrialDivision-CPU-Single";
}

void TrialDivisionStrategy::addKnownPrime(const std::vector<uint32_t>& prime_limbs) {
    known_primes.push_back(prime_limbs);
}

// A simple CPU fallback. Since implementing big integer division manually in C++ 
// without a library is complex, and the user specifically requested GPU, 
// we leave this as a stub that always returns false (or we could use GMP if available).
// This is just to satisfy the interface. The real work is in GpuTrialDivisionStrategy.
bool TrialDivisionStrategy::testSingleCandidate(
    const std::vector<uint32_t>& candidate,
    uint32_t bit_length) 
{
    // STUB: Real trial division with big ints is meant for the GPU via CGBN.
    return false;
}
