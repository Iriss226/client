#pragma once

#include <atomic>
#include <cstdint>

struct ShmRing {
    static constexpr uint32_t CAPACITY = 1024;

    void init() {
        head.store(0, std::memory_order_relaxed);
        tail.store(0, std::memory_order_relaxed);
    }

    bool push(uint64_t value) {
        const uint32_t head_value = head.load(std::memory_order_relaxed);
        const uint32_t tail_value = tail.load(std::memory_order_acquire);
        if (head_value - tail_value == CAPACITY) {
            return false;
        }

        buf[head_value & (CAPACITY - 1)] = value;
        head.store(head_value + 1, std::memory_order_release);
        return true;
    }

    bool pop(uint64_t& out) {
        const uint32_t tail_value = tail.load(std::memory_order_relaxed);
        const uint32_t head_value = head.load(std::memory_order_acquire);
        if (tail_value == head_value) {
            return false;
        }

        out = buf[tail_value & (CAPACITY - 1)];
        tail.store(tail_value + 1, std::memory_order_release);
        return true;
    }

    alignas(64) std::atomic<uint32_t> head{0};
    alignas(64) std::atomic<uint32_t> tail{0};
    uint64_t buf[CAPACITY]{};
};
