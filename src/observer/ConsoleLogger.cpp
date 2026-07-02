/***
 * ConsoleLogger.cpp – Színes Konzol Observer Implementáció
 ***/

#include "observer/ConsoleLogger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
static void enableAnsiColors() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#else
static void enableAnsiColors() {}
#endif

// ANSI szín kódok
namespace Color {
    const char* RESET   = "\033[0m";
    const char* GREEN   = "\033[32m";
    const char* YELLOW  = "\033[33m";
    const char* CYAN    = "\033[36m";
    const char* MAGENTA = "\033[35m";
    const char* BOLD    = "\033[1m";
    const char* DIM     = "\033[2m";
}

ConsoleLogger::ConsoleLogger(bool verbose) : verbose_(verbose) {
    enableAnsiColors();
}

void ConsoleLogger::onPrimeFound(const std::string& prime_dec,
                                  uint32_t bit_length,
                                  uint64_t prime_index) {
    std::string display_dec = prime_dec;
    if (display_dec.length() > 32) {
        display_dec = display_dec.substr(0, 16) + "..." +
                      display_dec.substr(display_dec.length() - 16);
    }

    std::cout << Color::BOLD << Color::GREEN
              << "  [PRIME #" << prime_index << "] "
              << Color::RESET << Color::CYAN
              << display_dec
              << Color::DIM << " (" << bit_length << " bit)"
              << Color::RESET << std::endl;
}

void ConsoleLogger::onProgress(uint64_t candidates_tested,
                                uint64_t primes_found,
                                double elapsed_seconds,
                                double candidates_per_sec) {
    if (!verbose_) return;

    std::cout << Color::DIM
              << "  [" << std::fixed << std::setprecision(1) << elapsed_seconds << "s] "
              << Color::RESET
              << "Tesztelve: " << candidates_tested
              << " | Primek: " << Color::GREEN << primes_found << Color::RESET
              << " | Sebesseg: " << Color::YELLOW
              << std::fixed << std::setprecision(0) << candidates_per_sec
              << " jelolt/s" << Color::RESET
              << "\r" << std::flush;
}

void ConsoleLogger::onSearchComplete(uint64_t total_tested,
                                      uint64_t total_found,
                                      double total_seconds) {
    std::cout << std::endl;
    std::cout << Color::BOLD << Color::MAGENTA
              << "════════════════════════════════════════════" << std::endl
              << "  Kereses befejezve!" << std::endl
              << Color::RESET
              << "  Tesztelve:  " << total_tested << " jelolt" << std::endl
              << "  Talalt:     " << Color::GREEN << total_found << " prim" << Color::RESET << std::endl
              << "  Ido:        " << std::fixed << std::setprecision(2) << total_seconds << " mp" << std::endl
              << "  Sebesseg:   " << std::fixed << std::setprecision(0)
              << (total_seconds > 0 ? total_tested / total_seconds : 0)
              << " jelolt/s" << std::endl
              << Color::BOLD << Color::MAGENTA
              << "════════════════════════════════════════════"
              << Color::RESET << std::endl;
}
