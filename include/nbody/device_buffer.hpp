#pragma once
#include <cstddef>
#include <cuda_runtime.h>
#include <stdexcept>
#include <utility>

// RAII wrapper around cudaMalloc/cudaFree, used by Simulator and the solvers.
template <typename T>
class DeviceBuffer {
public:
    DeviceBuffer() = default;
    explicit DeviceBuffer(std::size_t count) { resize(count); }
    ~DeviceBuffer() { cudaFree(m_ptr); }
    DeviceBuffer(const DeviceBuffer&) = delete;
    DeviceBuffer& operator=(const DeviceBuffer&) = delete;
    DeviceBuffer(DeviceBuffer&& other) noexcept
        : m_ptr(other.m_ptr), m_count(other.m_count) {
        other.m_ptr = nullptr;
        other.m_count = 0;
    }
    DeviceBuffer& operator=(DeviceBuffer&& other) noexcept {
        if (this != &other) {
            cudaFree(m_ptr);
            m_ptr = other.m_ptr;
            m_count = other.m_count;
            other.m_ptr = nullptr;
            other.m_count = 0;
        }
        return *this;
    }

    T* get() const { return m_ptr; }
    std::size_t size() const { return m_count; }

    void resize(std::size_t count) {
        if (count == m_count) {
            return;
        }

        T* new_ptr = nullptr;
        if (count != 0) {
            cudaError_t error = cudaMalloc(reinterpret_cast<void**>(&new_ptr),
                                           count * sizeof(T));
            if (error != cudaSuccess) {
                throw std::runtime_error(cudaGetErrorString(error));
            }

            const std::size_t copy_count = count < m_count ? count : m_count;
            if (m_ptr != nullptr && copy_count != 0) {
                error = cudaMemcpy(new_ptr, m_ptr, copy_count * sizeof(T),
                                   cudaMemcpyDeviceToDevice);
                if (error != cudaSuccess) {
                    cudaFree(new_ptr);
                    throw std::runtime_error(cudaGetErrorString(error));
                }
            }
        }

        cudaFree(m_ptr);
        m_ptr = new_ptr;
        m_count = count;
    }

    void copyFromHost(const T* h_data, std::size_t count) {
        if (count > m_count) {
            throw std::runtime_error("DeviceBuffer: copyFromHost size exceeds buffer size.");
        }
        cudaError_t error = cudaMemcpy(m_ptr, h_data, count * sizeof(T),
                                       cudaMemcpyHostToDevice);
        if (error != cudaSuccess) {
            throw std::runtime_error(cudaGetErrorString(error));
        }
    }

    void copyToHost(T* h_data, std::size_t count) const {
        if (count > m_count) {
            throw std::runtime_error("DeviceBuffer: copyToHost size exceeds buffer size.");
        }
        cudaError_t error = cudaMemcpy(h_data, m_ptr, count * sizeof(T),
                                       cudaMemcpyDeviceToHost);
        if (error != cudaSuccess) {
            throw std::runtime_error(cudaGetErrorString(error));
        }
    }

    void swap(DeviceBuffer& other) noexcept {
        std::swap(m_ptr, other.m_ptr);
        std::swap(m_count, other.m_count);
    }


private:
    T* m_ptr = nullptr;
    std::size_t m_count = 0;
};

