#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

struct SPSCRing {
    explicit SPSCRing(std::size_t capacity_pow2) : buffer_(capacity_pow2) {
        if (capacity_pow2 == 0 ||
            (capacity_pow2 & (capacity_pow2 - 1)) != 0) {
            throw std::invalid_argument("SPSCRing capacity must be a power of two");
        }
        mask_ = capacity_pow2 - 1;
    }

    SPSCRing(const SPSCRing&) = delete;
    SPSCRing& operator=(const SPSCRing&) = delete;

    bool push(std::uint64_t value) {
        const std::uint64_t head = head_.value.load(std::memory_order_relaxed);
        const std::uint64_t tail = tail_.value.load(std::memory_order_acquire);
        if (head - tail == buffer_.size()) {
            return false;
        }

        buffer_[head & mask_] = value;
        head_.value.store(head + 1, std::memory_order_release);
        return true;
    }

    bool pop(std::uint64_t& out) {
        const std::uint64_t tail = tail_.value.load(std::memory_order_relaxed);
        const std::uint64_t head = head_.value.load(std::memory_order_acquire);
        if (tail == head) {
            return false;
        }

        out = buffer_[tail & mask_];
        tail_.value.store(tail + 1, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return tail_.value.load(std::memory_order_acquire) ==
               head_.value.load(std::memory_order_acquire);
    }

    bool full() const {
        const std::uint64_t head = head_.value.load(std::memory_order_acquire);
        const std::uint64_t tail = tail_.value.load(std::memory_order_acquire);
        return head - tail == buffer_.size();
    }

private:
    struct alignas(64) PaddedIndex {
        std::atomic<std::uint64_t> value{0};
    };

    std::vector<std::uint64_t> buffer_;
    std::size_t mask_ = 0;
    PaddedIndex head_;
    PaddedIndex tail_;
};
