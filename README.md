# LiptaiKripto – CUDA BigInt Prímszám-kereső

GPU-gyorsított prímszám-kereső alkalmazás, amely az NVIDIA [CGBN](https://github.com/NVlabs/CGBN) (Cooperative Groups Big Numbers) könyvtárat használja max **4096 bites** nagy számok kezelésére.

## ✨ Jellemzők

- **4096 bites BigInt aritmetika** CGBN-nel (32 GPU thread / szám)
- **Miller-Rabin prímteszt** windowed Montgomery hatványozással
- **Batch feldolgozás** – egyszerre több ezer jelölt párhuzamos tesztelése
- **Design Patternek**: Strategy, Factory, Observer, Builder, RAII

## 🏗️ Architektúra

```
┌──────────────── Host (CPU) ────────────────┐
│  main.cpp → SearchConfig (Builder)         │
│           → PrimeSearchFactory (Factory)   │
│           → PrimeSearcher                  │
│           → ISearchObserver (Observer)     │
│              ├── ConsoleLogger             │
│              └── ResultFileWriter          │
└──────────────────────┬─────────────────────┘
                       │ cudaMemcpy
┌──────────────── Device (GPU) ──────────────┐
│  IPrimalityStrategy (Strategy)             │
│    ├── TrialDivisionStrategy (CPU)         │
│    └── MillerRabinGpuStrategy              │
│         └── CGBN 4096-bit kernels          │
│              ├── kernel_miller_rabin       │
│              └── windowed Montgomery powm  │
│  CudaMemory<T> (RAII)                     │
└────────────────────────────────────────────┘
```

## 📋 Előfeltételek

- **CUDA Toolkit 12.x**
- **CMake 3.18+**
- **Visual Studio 2022** (C++ desktop development workload)
- **NVIDIA GPU** (Compute Capability 7.0+)

## 🔧 Build

```bash
# Klónozás submodule-okkal
git clone --recursive <repo-url>
cd liptaikripto

# Build
mkdir build && cd build
cmake .. -DCMAKE_CUDA_ARCHITECTURES=86
cmake --build . --config Release
```

## 🚀 Használat

```bash
# Alapértelmezett keresés: 1024 bites random prímek
./LiptaiKripto

# Testreszabott keresés
./LiptaiKripto --bits 2048 --batch-size 50000 --rounds 25 --output primes.txt
```

### Parancssori opciók

| Opció | Leírás | Alapértelmezett |
|-------|--------|-----------------|
| `--bits` | Keresett prímek bit mérete | 1024 |
| `--count` | Hány prímet keressen | 10 |
| `--batch-size` | GPU batch méret | 10000 |
| `--rounds` | Miller-Rabin iterációk | 20 |
| `--output` | Kimeneti fájl | (nincs) |
| `--strategy` | Stratégia: `millerrabin` / `trial` | `millerrabin` |

## 📁 Projekt Struktúra

```
liptaikripto/
├── CMakeLists.txt          # Build konfiguráció
├── external/CGBN/          # CGBN könyvtár (git submodule)
├── include/                # Header fájlok
│   ├── config/             # CGBN paraméterek
│   ├── primality/          # Prímteszt stratégiák
│   ├── search/             # Keresési motor
│   ├── observer/           # Observer pattern
│   ├── cuda/               # RAII GPU memória
│   └── utils/              # Konverziós segédletek
├── src/                    # Implementációk
└── tests/                  # Unit tesztek
```

## 📜 Licensz

MIT
