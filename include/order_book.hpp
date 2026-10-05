#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>

struct Book {
    void add(uint64_t id, char side, double px, uint32_t qty) {
        cancel(id);
        if ((side != 'B' && side != 'S') || qty == 0) {
            return;
        }

        auto& levels = side == 'B' ? bids_ : asks_;
        levels[px] += qty;
        orders_[id] = Order{side, px, qty};
    }

    void cancel(uint64_t id) {
        const auto order = orders_.find(id);
        if (order == orders_.end()) {
            return;
        }
        auto& levels = order->second.side == 'B' ? bids_ : asks_;
        const auto level = levels.find(order->second.price);
        if (level != levels.end()) {
            if (level->second <= order->second.quantity) {
                levels.erase(level);
            } else {
                level->second -= order->second.quantity;
            }
        }
        orders_.erase(order);
    }

    double best_bid() const {
        return bids_.empty() ? 0.0 : bids_.rbegin()->first;
    }

    double best_ask() const {
        return asks_.empty() ? 0.0 : asks_.begin()->first;
    }

private:
    struct Order {
        char side;
        double price;
        uint32_t quantity;
    };

    std::map<double, uint64_t> bids_;
    std::map<double, uint64_t> asks_;
    std::unordered_map<uint64_t, Order> orders_;
};

struct SymMap {
    void put(const char* symbol, uint64_t id) {
        if (symbol != nullptr) {
            symbols_[symbol] = id;
        }
    }

    uint64_t get(const char* symbol) const {
        if (symbol == nullptr) {
            return static_cast<uint64_t>(-1);
        }
        const auto entry = symbols_.find(symbol);
        return entry == symbols_.end() ? static_cast<uint64_t>(-1) : entry->second;
    }

private:
    std::unordered_map<std::string, uint64_t> symbols_;
};
