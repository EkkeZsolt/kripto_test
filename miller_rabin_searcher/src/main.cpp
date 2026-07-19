/***
 * main.cpp – LiptaiKripto CLI Entry Point
 *
 * Parancssori interfész a prímszám-keresőhöz.
 * Használat: LiptaiKripto [opciók]
 *
 * RTX 3090 + Ryzen 9 5950X optimalizált verzió
 ***/

#include "search/SearchConfig.h"
#include "search/PrimeSearchFactory.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cuda_runtime.h>
#include <omp.h>

// ────────────────────────────────────────────────
// GPU info kiírása
// ────────────────────────────────────────────────
void printGpuInfo() {
    int device_count = 0;
    cudaGetDeviceCount(&device_count);

    if (device_count == 0) {
        std::cerr << "HIBA: Nem talalhato CUDA-kompatibilis GPU!" << std::endl;
        return;
    }

    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);

    std::cout << "\033[36m"
              << "  GPU: " << prop.name << "\n"
              << "  Compute Capability: " << prop.major << "." << prop.minor << "\n"
              << "  SM-ek: " << prop.multiProcessorCount << "\n"
              << "  VRAM: " << (prop.totalGlobalMem / (1024*1024)) << " MB\n"
              << "  Max Threads/SM: " << prop.maxThreadsPerMultiProcessor << "\n"
              << "\033[0m" << std::endl;
}

// ────────────────────────────────────────────────
// CPU info kiírása
// ────────────────────────────────────────────────
void printCpuInfo() {
    std::cout << "\033[33m"
              << "  CPU szalak (OpenMP): " << omp_get_max_threads() << "\n"
              << "\033[0m" << std::endl;
}

// ────────────────────────────────────────────────
// Banner
// ────────────────────────────────────────────────
void printBanner() {
    std::cout << "\033[1m\033[35m" << R"(
  ╦   ╦╔═╗╔╦╗╔═╗╦  ╦╔═╗╦═╗╦╔═╗╔╦╗╔═╗
  ║   ║╠═╝ ║ ╠═╣║  ║╠╩╗╠╦╝║╠═╝ ║ ║ ║
  ╩═╝╩╩   ╩ ╩ ╩╩  ╩╩ ╩╩╚═╩╩   ╩ ╚═╝
  )" << "\033[0m"
    << "\033[2m  CUDA BigInt Primszam-kereso | CGBN 4096-bit | RTX 3090 + Zen 3 Optimized\033[0m\n"
    << std::endl;
}

// ────────────────────────────────────────────────
// Használat
// ────────────────────────────────────────────────
void printUsage() {
    std::cout << "Hasznalat: LiptaiKripto [opciok]\n\n"
              << "Opciok:\n"
              << "  --bits N        Primek bit merete (default: 256, max: 4096)\n"
              << "  --count N       Hany primet keressen (default: 0 = vegtelen)\n"
              << "  --batch-size N  GPU batch meret (default: auto, 20 GiB VRAM alapjan)\n"
              << "  --rounds N      Miller-Rabin iteraciok (default: 20)\n"
              << "  --strategy S    'millerrabin' vagy 'trial' (default: mr)\n"
              << "  --output FILE   Kimeneti fajl (default: primes.txt)\n"
              << "  --start NUM     Szekvencialis kezdoertek decimalis formatumban\n"
              << "  --no-prefilter  Trial division eloszures kikapcsolasa\n"
              << "  --threads N     OpenMP szalak szama (default: osszes mag)\n"
              << "  --quiet         Csak primeket ir ki\n"
              << "  --help          Sugo\n"
              << std::endl;
}

// ────────────────────────────────────────────────
// Main
// ────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    printBanner();

    SearchConfig::Builder builder;
    bool show_help = false;

    // Parancssori argumentumok feldolgozása
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            show_help = true;
        }
        else if (arg == "--bits" && i + 1 < argc) {
            uint32_t bits = std::stoul(argv[++i]);
            if (bits > 4096 || bits < 32 || bits % 32 != 0) {
                std::cerr << "HIBA: --bits erteke 32 es 4096 kozott legyen, "
                          << "32 tobbszorose." << std::endl;
                return 1;
            }
            builder.setBitLength(bits);
        }
        else if (arg == "--count" && i + 1 < argc) {
            builder.setPrimeCount(std::stoul(argv[++i]));
        }
        else if (arg == "--batch-size" && i + 1 < argc) {
            builder.setBatchSize(std::stoul(argv[++i]));
        }
        else if (arg == "--rounds" && i + 1 < argc) {
            builder.setMillerRabinRounds(std::stoul(argv[++i]));
        }
        else if (arg == "--strategy" && i + 1 < argc) {
            builder.setStrategy(argv[++i]);
        }
        else if (arg == "--output" && i + 1 < argc) {
            builder.setOutputFile(argv[++i]);
        }
        else if (arg == "--no-prefilter") {
            builder.setPrefilter(false);
        }
        else if (arg == "--quiet") {
            builder.setVerbose(false);
        }
        else if (arg == "--start" && i + 1 < argc) {
            builder.setStartNumber(argv[++i]);
        }
        else if (arg == "--threads" && i + 1 < argc) {
            int num_threads = std::stoi(argv[++i]);
            if (num_threads > 0) {
                omp_set_num_threads(num_threads);
            }
        }
        else {
            std::cerr << "Ismeretlen opcio: " << arg << std::endl;
            printUsage();
            return 1;
        }
    }

    if (show_help) {
        printUsage();
        return 0;
    }

    // GPU & CPU info
    printGpuInfo();
    printCpuInfo();

    try {
        // Konfiguráció összeállítása
        auto config = builder.build();

        // Factory létrehozza a keresőt
        auto searcher = PrimeSearchFactory::create(config);

        // Keresés indítása
        auto primes = searcher->search();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\033[31mHIBA: " << e.what() << "\033[0m" << std::endl;
        return 1;
    }
}
