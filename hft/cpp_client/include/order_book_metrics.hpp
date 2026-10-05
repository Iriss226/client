#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace order_book {

struct PriceLevel {
    double price;
    double size;
};

struct Snapshot {
    std::vector<PriceLevel> bids;
    std::vector<PriceLevel> asks;
};

struct Metrics {
    double best_bid;
    double best_ask;
    double mid;
    double spread;
    double microprice;
    double imbalance;
};

inline std::optional<Metrics> calculate_metrics(const Snapshot& snapshot) {
    if (snapshot.bids.empty() || snapshot.asks.empty()) {
        return std::nullopt;
    }

    const auto valid_level = [](const PriceLevel& level) {
        return std::isfinite(level.price) && level.price > 0.0 &&
               std::isfinite(level.size) && level.size > 0.0;
    };
    if (!std::all_of(snapshot.bids.begin(), snapshot.bids.end(), valid_level) ||
        !std::all_of(snapshot.asks.begin(), snapshot.asks.end(), valid_level)) {
        return std::nullopt;
    }

    const auto best_bid_level = std::max_element(
        snapshot.bids.begin(), snapshot.bids.end(),
        [](const PriceLevel& lhs, const PriceLevel& rhs) {
            return lhs.price < rhs.price;
        });
    const auto best_ask_level = std::min_element(
        snapshot.asks.begin(), snapshot.asks.end(),
        [](const PriceLevel& lhs, const PriceLevel& rhs) {
            return lhs.price < rhs.price;
        });

    if (best_bid_level->price > best_ask_level->price) {
        return std::nullopt;
    }

    const double bid_size = best_bid_level->size;
    const double ask_size = best_ask_level->size;
    const double size_scale = std::max(bid_size, ask_size);
    const double scaled_bid_size = bid_size / size_scale;
    const double scaled_ask_size = ask_size / size_scale;
    const double scaled_total = scaled_bid_size + scaled_ask_size;
    const double best_bid = best_bid_level->price;
    const double best_ask = best_ask_level->price;
    const double mid = best_bid + (best_ask - best_bid) / 2.0;
    const double spread = best_ask - best_bid;
    const double microprice =
        (best_ask * scaled_bid_size + best_bid * scaled_ask_size) / scaled_total;
    const double imbalance =
        (scaled_bid_size - scaled_ask_size) / scaled_total;

    if (!std::isfinite(mid) || !std::isfinite(spread) ||
        !std::isfinite(microprice) || !std::isfinite(imbalance)) {
        return std::nullopt;
    }

    return Metrics{best_bid, best_ask, mid, spread, microprice, imbalance};
}

}  // namespace order_book
