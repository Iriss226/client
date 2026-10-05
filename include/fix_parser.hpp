#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

struct NewOrder {
    const char* clordid;
    int clordid_len;
    char symbol[16];
    char side;
    uint32_t qty;
    double price;
};

namespace fix_parser_detail {

inline bool parse_uint(const char* text, int length, uint64_t& value) {
    if (length <= 0) {
        return false;
    }
    value = 0;
    for (int i = 0; i < length; ++i) {
        const char c = text[i];
        if (c < '0' || c > '9') {
            return false;
        }
        const uint64_t digit = static_cast<uint64_t>(c - '0');
        if (value > (std::numeric_limits<uint64_t>::max() - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
    }
    return true;
}

inline bool parse_price(const char* text, int length, double& price) {
    if (length <= 0) {
        return false;
    }

    int i = 0;
    bool negative = false;
    if (text[i] == '-' || text[i] == '+') {
        negative = text[i] == '-';
        if (++i == length) {
            return false;
        }
    }

    long double value = 0.0L;
    bool has_digit = false;
    while (i < length && text[i] >= '0' && text[i] <= '9') {
        has_digit = true;
        value = value * 10.0L + (text[i++] - '0');
        if (!std::isfinite(value)) {
            return false;
        }
    }
    if (i < length && text[i] == '.') {
        ++i;
        long double place = 0.1L;
        while (i < length && text[i] >= '0' && text[i] <= '9') {
            has_digit = true;
            value += (text[i++] - '0') * place;
            place *= 0.1L;
        }
    }
    if (!has_digit || i != length) {
        return false;
    }
    if (negative) {
        value = -value;
    }
    price = static_cast<double>(value);
    return std::isfinite(price) && price > 0.0;
}

}  // namespace fix_parser_detail

inline bool parse_new_order(const char* buf, int len, NewOrder& out) {
    if (buf == nullptr || len <= 0) {
        return false;
    }

    NewOrder parsed{};
    bool has_type = false;
    bool has_clordid = false;
    bool has_symbol = false;
    bool has_side = false;
    bool has_qty = false;
    bool has_price = false;
    int position = 0;

    while (position < len) {
        const int field_start = position;
        while (position < len && buf[position] >= '0' && buf[position] <= '9') {
            ++position;
        }
        if (position == field_start || position >= len || buf[position] != '=') {
            return false;
        }

        uint64_t tag = 0;
        if (!fix_parser_detail::parse_uint(buf + field_start,
                                           position - field_start, tag)) {
            return false;
        }
        const int value_start = ++position;
        while (position < len && buf[position] != '\x01') {
            ++position;
        }
        if (position == len || position == value_start) {
            return false;
        }
        const int value_length = position - value_start;
        const char* value = buf + value_start;
        ++position;

        switch (tag) {
            case 35:
                if (has_type || value_length != 1 || value[0] != 'D') {
                    return false;
                }
                has_type = true;
                break;
            case 11:
                if (has_clordid) return false;
                parsed.clordid = value;
                parsed.clordid_len = value_length;
                has_clordid = true;
                break;
            case 55:
                if (has_symbol || value_length >= static_cast<int>(sizeof(parsed.symbol))) {
                    return false;
                }
                std::memcpy(parsed.symbol, value, static_cast<std::size_t>(value_length));
                parsed.symbol[value_length] = '\0';
                has_symbol = true;
                break;
            case 54:
                if (has_side || value_length != 1 ||
                    (value[0] != '1' && value[0] != '2')) {
                    return false;
                }
                parsed.side = value[0];
                has_side = true;
                break;
            case 38: {
                uint64_t quantity = 0;
                if (has_qty || !fix_parser_detail::parse_uint(value, value_length, quantity) ||
                    quantity == 0 || quantity > std::numeric_limits<uint32_t>::max()) {
                    return false;
                }
                parsed.qty = static_cast<uint32_t>(quantity);
                has_qty = true;
                break;
            }
            case 44:
                if (has_price ||
                    !fix_parser_detail::parse_price(value, value_length, parsed.price)) {
                    return false;
                }
                has_price = true;
                break;
            default:
                break;
        }
    }

    if (!has_type || !has_clordid || !has_symbol || !has_side ||
        !has_qty || !has_price) {
        return false;
    }
    out = parsed;
    return true;
}
