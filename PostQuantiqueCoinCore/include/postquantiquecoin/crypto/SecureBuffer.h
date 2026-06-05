#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace pqc {
class SecureBuffer {
public:
    SecureBuffer() = default;
    explicit SecureBuffer(size_t size);
    explicit SecureBuffer(const std::vector<uint8_t>& data);
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;
    SecureBuffer(SecureBuffer&& other) noexcept;
    SecureBuffer& operator=(SecureBuffer&& other) noexcept;
    ~SecureBuffer();
    void Wipe();
    uint8_t* Data();
    const uint8_t* Data() const;
    size_t Size() const;
    bool Empty() const;
    std::vector<uint8_t> ToVectorCopy() const;
private:
    std::vector<uint8_t> bytes_;
    bool locked_{false};
    void TryLock();
    void TryUnlock();
};
}
