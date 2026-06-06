#include "postquantiquecoin/crypto/SecureBuffer.h"
#include <algorithm>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif

namespace pqc {
namespace {
void SecureWipe(uint8_t* p, size_t n) {
    if (!p || n == 0) return;
#ifdef _WIN32
    SecureZeroMemory(p, n);
#else
    volatile uint8_t* v = p;
    while (n--) *v++ = 0;
#endif
}
}

SecureBuffer::SecureBuffer(size_t size) : bytes_(size) { TryLock(); }
SecureBuffer::SecureBuffer(const std::vector<uint8_t>& data) : bytes_(data) { TryLock(); }
SecureBuffer::SecureBuffer(std::vector<uint8_t>&& data, bool) : bytes_(std::move(data)) { TryLock(); }
SecureBuffer SecureBuffer::FromVectorMove(std::vector<uint8_t>&& data) { return SecureBuffer(std::move(data), true); }
SecureBuffer::SecureBuffer(SecureBuffer&& other) noexcept : bytes_(std::move(other.bytes_)), locked_(false) { other.locked_ = false; TryLock(); }
SecureBuffer& SecureBuffer::operator=(SecureBuffer&& other) noexcept {
    if (this != &other) {
        Wipe();
        TryUnlock();
        bytes_ = std::move(other.bytes_);
        locked_ = false;
        other.locked_ = false;
        TryLock();
    }
    return *this;
}
SecureBuffer::~SecureBuffer() { Wipe(); TryUnlock(); }
void SecureBuffer::TryLock() {
    if (bytes_.empty()) return;
#ifdef _WIN32
    locked_ = VirtualLock(bytes_.data(), bytes_.size()) != 0;
#else
    locked_ = mlock(bytes_.data(), bytes_.size()) == 0;
#endif
}
void SecureBuffer::TryUnlock() {
    if (!locked_ || bytes_.empty()) return;
#ifdef _WIN32
    VirtualUnlock(bytes_.data(), bytes_.size());
#else
    munlock(bytes_.data(), bytes_.size());
#endif
    locked_ = false;
}
void SecureBuffer::Wipe() { SecureWipe(bytes_.data(), bytes_.size()); }
uint8_t* SecureBuffer::Data() { return bytes_.data(); }
const uint8_t* SecureBuffer::Data() const { return bytes_.data(); }
size_t SecureBuffer::Size() const { return bytes_.size(); }
bool SecureBuffer::Empty() const { return bytes_.empty(); }
SecureVectorView SecureBuffer::View() const { return {bytes_.data(), bytes_.size()}; }
std::vector<uint8_t> SecureBuffer::ToVectorCopy() const { return bytes_; }
std::vector<uint8_t> SecureBuffer::CopyToTemporary() const { return ToVectorCopy(); }
}
