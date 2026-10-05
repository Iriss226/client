#pragma once

#include <cstdint>
#include <deque>

struct RollingCounter {
    explicit RollingCounter(uint64_t window_ns) : window_ns_(window_ns) {}

    void add(uint64_t ts_ns) {
        timestamps_.push_back(ts_ns);
    }

    uint64_t count(uint64_t now_ns) {
        if (now_ns < window_ns_) {
            return static_cast<uint64_t>(timestamps_.size());
        }

        const uint64_t cutoff = now_ns - window_ns_;
        while (!timestamps_.empty() && timestamps_.front() <= cutoff) {
            timestamps_.pop_front();
        }
        return static_cast<uint64_t>(timestamps_.size());
    }

private:
    uint64_t window_ns_;
    std::deque<uint64_t> timestamps_;
};
