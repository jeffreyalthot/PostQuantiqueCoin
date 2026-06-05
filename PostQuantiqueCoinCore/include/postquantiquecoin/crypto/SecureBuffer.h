#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace pqc {
struct SecureVectorView {
    const uint8_t* data{nullptr};
    size_t size{0};
};

class SecureBuffer {
public:
    class WipeOnScopeExit {
    public:
        explicit WipeOnScopeExit(SecureBuffer& buffer) : buffer_(buffer) {}
        WipeOnScopeExit(const WipeOnScopeExit&) = delete;
        WipeOnScopeExit& operator=(const WipeOnScopeExit&) = delete;
        ~WipeOnScopeExit() { buffer_.Wipe(); }
    private:
        SecureBuffer& buffer_;
    };

    SecureBuffer() = default;
    explicit SecureBuffer(size_t size);
    explicit SecureBuffer(const std::vector<uint8_t>& data);
    static SecureBuffer FromVectorMove(std::vector<uint8_t>&& data);
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
    SecureVectorView View() const;
    std::vector<uint8_t> ToVectorCopy() const;
    std::vector<uint8_t> CopyToTemporary() const;
private:
    std::vector<uint8_t> bytes_;
    bool locked_{false};
    explicit SecureBuffer(std::vector<uint8_t>&& data, bool takeOwnership);
    void TryLock();
    void TryUnlock();
};
}
