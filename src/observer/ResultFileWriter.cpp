/***
 * ResultFileWriter.cpp – Fájl Observer Implementáció
 ***/

#include "observer/ResultFileWriter.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

ResultFileWriter::ResultFileWriter(const std::string& filepath)
    : filepath_(filepath) {
    file_.open(filepath, std::ios::out | std::ios::app);
    if (!file_.is_open()) {
        std::cerr << "FIGYELMEZETES: Nem sikerult megnyitni: " << filepath << std::endl;
        return;
    }

    // Header írása
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    file_ << "# LiptaiKripto - Primszam kereses eredmenyek" << std::endl;
    file_ << "# Datum: " << std::ctime(&time);
    file_ << "# Format: index | bit_length | hex" << std::endl;
    file_ << "# ────────────────────────────────────────" << std::endl;
}

ResultFileWriter::~ResultFileWriter() {
    if (file_.is_open()) {
        file_.close();
    }
}

void ResultFileWriter::onPrimeFound(const std::string& prime_hex,
                                     uint32_t bit_length,
                                     uint64_t prime_index) {
    if (!file_.is_open()) return;

    file_ << prime_index << " | "
          << bit_length << " bit | "
          << prime_hex << std::endl;
    file_.flush();
}

void ResultFileWriter::onProgress(uint64_t, uint64_t, double, double) {
    // Fájlba nem írjuk a haladást
}

void ResultFileWriter::onSearchComplete(uint64_t total_tested,
                                         uint64_t total_found,
                                         double total_seconds) {
    if (!file_.is_open()) return;

    file_ << "# ────────────────────────────────────────" << std::endl;
    file_ << "# Osszesen tesztelve: " << total_tested << std::endl;
    file_ << "# Osszesen talalt:    " << total_found << std::endl;
    file_ << "# Ido:                " << std::fixed << std::setprecision(2)
          << total_seconds << " mp" << std::endl;
    file_.flush();
}
