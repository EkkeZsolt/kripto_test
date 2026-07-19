/***
 * CudaMemory.h – RAII GPU Memória Wrapper
 *
 * Design Pattern: RAII (Resource Acquisition Is Initialization)
 * Automatikusan kezeli a GPU memória allokációt és felszabadítást,
 * így nem kell manuálisan cudaFree-t hívni.
 *
 * Támogatja a CUDA Stream-es async másolást a CPU/GPU átfedéshez.
 ***/

#pragma once
#include <cuda_runtime.h>
#include <stdexcept>
#include <string>
#include <cstring>

/// RAII wrapper CUDA device memóriához
/// @tparam T Az elemek típusa a GPU memóriában
template<typename T>
class CudaMemory {
public:
    /// Allokál count darab T elemet a GPU-n
    explicit CudaMemory(size_t count) : count_(count) {
        if (count_ == 0) return;
        cudaError_t err = cudaMalloc(&d_ptr_, sizeof(T) * count_);
        if (err != cudaSuccess) {
            throw std::runtime_error(
                "cudaMalloc failed: " + std::string(cudaGetErrorString(err))
            );
        }
    }

    /// Destruktor – felszabadítja a GPU memóriát
    ~CudaMemory() {
        if (d_ptr_) {
            cudaFree(d_ptr_);
            d_ptr_ = nullptr;
        }
    }

    // ── Move semantics ──
    CudaMemory(CudaMemory&& other) noexcept
        : d_ptr_(other.d_ptr_), count_(other.count_) {
        other.d_ptr_ = nullptr;
        other.count_ = 0;
    }

    CudaMemory& operator=(CudaMemory&& other) noexcept {
        if (this != &other) {
            if (d_ptr_) cudaFree(d_ptr_);
            d_ptr_ = other.d_ptr_;
            count_ = other.count_;
            other.d_ptr_ = nullptr;
            other.count_ = 0;
        }
        return *this;
    }

    // ── No copy ──
    CudaMemory(const CudaMemory&) = delete;
    CudaMemory& operator=(const CudaMemory&) = delete;

    /// Host → Device másolás (szinkron)
    void copyToDevice(const T* host_data) {
        cudaError_t err = cudaMemcpy(
            d_ptr_, host_data, sizeof(T) * count_, cudaMemcpyHostToDevice
        );
        if (err != cudaSuccess) {
            throw std::runtime_error(
                "cudaMemcpy H2D failed: " + std::string(cudaGetErrorString(err))
            );
        }
    }

    /// Device → Host másolás (szinkron)
    void copyToHost(T* host_data) const {
        cudaError_t err = cudaMemcpy(
            host_data, d_ptr_, sizeof(T) * count_, cudaMemcpyDeviceToHost
        );
        if (err != cudaSuccess) {
            throw std::runtime_error(
                "cudaMemcpy D2H failed: " + std::string(cudaGetErrorString(err))
            );
        }
    }

    /// Host → Device async másolás (CUDA stream-es, nem blokkoló)
    /// FONTOS: host_data-nak pinned memóriának KELL lennie (cudaMallocHost)!
    void copyToDeviceAsync(const T* host_data, cudaStream_t stream) {
        cudaError_t err = cudaMemcpyAsync(
            d_ptr_, host_data, sizeof(T) * count_,
            cudaMemcpyHostToDevice, stream
        );
        if (err != cudaSuccess) {
            throw std::runtime_error(
                "cudaMemcpyAsync H2D failed: " + std::string(cudaGetErrorString(err))
            );
        }
    }

    /// Device → Host async másolás (CUDA stream-es, nem blokkoló)
    /// FONTOS: host_data-nak pinned memóriának KELL lennie (cudaMallocHost)!
    void copyToHostAsync(T* host_data, cudaStream_t stream) const {
        cudaError_t err = cudaMemcpyAsync(
            host_data, d_ptr_, sizeof(T) * count_,
            cudaMemcpyDeviceToHost, stream
        );
        if (err != cudaSuccess) {
            throw std::runtime_error(
                "cudaMemcpyAsync D2H failed: " + std::string(cudaGetErrorString(err))
            );
        }
    }

    /// GPU memóriát nullázza
    void zero() {
        if (d_ptr_ && count_ > 0) {
            cudaMemset(d_ptr_, 0, sizeof(T) * count_);
        }
    }

    /// @return Pointer a device memóriára
    T* get() const { return d_ptr_; }

    /// @return Elemek száma
    size_t count() const { return count_; }

    /// @return Memória méret byte-ban
    size_t bytes() const { return sizeof(T) * count_; }

private:
    T*     d_ptr_ = nullptr;
    size_t count_ = 0;
};

// ──────────────────────────────────────────────
// Pinned Host Memory RAII Wrapper
// A cudaMemcpyAsync-hoz pinned (page-locked) memória kell
// a maximális PCIe sávszélesség kihasználásához.
// ──────────────────────────────────────────────
template<typename T>
class PinnedMemory {
public:
    explicit PinnedMemory(size_t count) : count_(count) {
        if (count_ == 0) return;
        cudaError_t err = cudaMallocHost(&h_ptr_, sizeof(T) * count_);
        if (err != cudaSuccess) {
            throw std::runtime_error(
                "cudaMallocHost failed: " + std::string(cudaGetErrorString(err))
            );
        }
    }

    ~PinnedMemory() {
        if (h_ptr_) {
            cudaFreeHost(h_ptr_);
            h_ptr_ = nullptr;
        }
    }

    PinnedMemory(PinnedMemory&& other) noexcept
        : h_ptr_(other.h_ptr_), count_(other.count_) {
        other.h_ptr_ = nullptr;
        other.count_ = 0;
    }

    PinnedMemory& operator=(PinnedMemory&& other) noexcept {
        if (this != &other) {
            if (h_ptr_) cudaFreeHost(h_ptr_);
            h_ptr_ = other.h_ptr_;
            count_ = other.count_;
            other.h_ptr_ = nullptr;
            other.count_ = 0;
        }
        return *this;
    }

    PinnedMemory(const PinnedMemory&) = delete;
    PinnedMemory& operator=(const PinnedMemory&) = delete;

    T* get() const { return h_ptr_; }
    size_t count() const { return count_; }
    size_t bytes() const { return sizeof(T) * count_; }

private:
    T*     h_ptr_ = nullptr;
    size_t count_ = 0;
};
