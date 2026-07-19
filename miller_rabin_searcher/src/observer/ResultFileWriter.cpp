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
}

ResultFileWriter::~ResultFileWriter() {
    if (file_.is_open()) {
        file_.close();
    }
}

void ResultFileWriter::onPrimeFound(const std::string& prime_dec,
                                     uint32_t bit_length,
                                     uint64_t prime_index) {
    if (!file_.is_open()) return;

    file_ << prime_dec << std::endl;
    file_.flush();
}

void ResultFileWriter::onProgress(uint64_t, uint64_t, double, double) {
    // Fájlba nem írjuk a haladást
}

void ResultFileWriter::onSearchComplete(uint64_t total_tested,
                                         uint64_t total_found,
                                         double total_seconds) {
    // Fájlba nem írunk statisztikát, csak a prímeket tartalmazza
}
