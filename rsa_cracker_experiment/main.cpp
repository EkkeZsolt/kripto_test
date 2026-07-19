#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

void run_powershell(const std::string& command) {
    std::string full_cmd = "powershell -Command \"" + command + "\"";
    std::system(full_cmd.c_str());
}

int main() {
    std::cout << "\033[1m\033[36m"
              << "========================================================\n"
              << "   RSA-129 Cracker Experiment (YAFU / GNFS Integration)\n"
              << "========================================================\n"
              << "\033[0m" << std::endl;

    std::cout << "A kiserlet egy 129 szamjegyu RSA modulus (RSA-129) feltoreset mutatja be." << std::endl;
    std::cout << "Keresett modulus: 114381625757888867669235779976146612010218296721242362562561842935706935245733897830597123563958705058989075147599290026879543541\n" << std::endl;

    std::string yafu_url = "https://github.com/bbuhrow/yafu/releases/download/v3.1.9/yafu-windows-avx2.exe";
    std::string yafu_dir = "yafu_bin";
    std::string yafu_exe = yafu_dir + "\\yafu-avx2.exe";

    // 1. Letöltés
    if (!fs::exists(yafu_exe)) {
        std::cout << "[*] YAFU akadémiai szoftver (v3.1.9 AVX2) letöltése a GitHub-ról..." << std::endl;
        
        if (!fs::exists(yafu_dir)) {
            fs::create_directory(yafu_dir);
        }

        run_powershell("Invoke-WebRequest -Uri '" + yafu_url + "' -OutFile '" + yafu_exe + "' -UseBasicParsing");
    } else {
        std::cout << "[*] YAFU mar telepitve van." << std::endl;
    }

    if (!fs::exists(yafu_exe)) {
        std::cerr << "HIBA: YAFU letöltése sikertelen. Ellenőrizd a mappát és az internetkapcsolatot!" << std::endl;
        return 1;
    }

    // 2. Futtatás
    std::cout << "\n[*] YAFU elinditasa a Ryzen 9 5950X maximalis kihasznalasaval (32 szál)...\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // A GNFS (General Number Field Sieve) futtatása a YAFU-n keresztül
    std::string rsa_129 = "114381625757888867669235779976146612010218296721242362562561842935706935245733897830597123563958705058989075147599290026879543541";
    
    // Az "siqs()" parancs használata teljesen kihagyja a felesleges előteszteket (ECM), 
    // és mivel a YAFU önmagában tartalmazza a SIQS-t, nem lesz szüksége a külső (és hiányzó) GGNFS binárisokra!
    std::string command = "cd " + yafu_dir + " && yafu-avx2.exe \"siqs(" + rsa_129 + ")\" -threads 32";
    
    std::cout << "> " << command << "\n\n";
    
    // Rendszerhívás, aminek a kimenetét a konzolra irányítjuk
    std::system(command.c_str());

    std::cout << "\n========================================================" << std::endl;
    std::cout << "[*] Befejezve! Ha feljebb gorgetsz a logban, lathatod az eredmenyeket." << std::endl;
    std::cout << "========================================================" << std::endl;

    return 0;
}
