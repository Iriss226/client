#include <iomanip>
#include <iostream>
#include <cstddef>
#include <vector>

#include "order_book_metrics.hpp"

int main() {
    const std::vector<order_book::Snapshot> snapshots{
        {{{100.00, 12.0}, {99.99, 20.0}}, {{100.02, 8.0}, {100.03, 15.0}}},
        {{{100.01, 18.0}, {100.00, 10.0}}, {{100.03, 7.0}, {100.04, 16.0}}},
        {{{100.02, 9.0}, {100.01, 14.0}}, {{100.04, 19.0}, {100.05, 11.0}}},
        {{{100.00, 6.0}, {99.99, 22.0}}, {{100.02, 17.0}, {100.03, 12.0}}},
        {{{100.03, 21.0}, {100.02, 13.0}}, {{100.05, 5.0}, {100.06, 10.0}}},
    };

    std::cout << std::fixed << std::setprecision(4)
              << "tick  best_bid  best_ask      mid  spread  microprice      OBI\n";

    for (std::size_t tick = 0; tick < snapshots.size(); ++tick) {
        const auto metrics = order_book::calculate_metrics(snapshots[tick]);
        if (!metrics) {
            std::cerr << "Invalid order-book snapshot at tick " << tick << '\n';
            return 1;
        }

        std::cout << std::setw(4) << tick << "  "
                  << std::setw(8) << metrics->best_bid << "  "
                  << std::setw(8) << metrics->best_ask << "  "
                  << std::setw(8) << metrics->mid << "  "
                  << std::setw(6) << metrics->spread << "  "
                  << std::setw(10) << metrics->microprice << "  "
                  << std::setw(7) << metrics->imbalance << '\n';
    }
}
