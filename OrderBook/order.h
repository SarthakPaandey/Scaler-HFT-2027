#pragma once

#include <cstdint>

struct Order {
    uint64_t order_id;
    bool is_buy;
    double price;
    uint64_t quantity;
    uint64_t timestamp_ns;
    
    Order(uint64_t id, bool buy, double p, uint64_t qty, uint64_t ts)
        : order_id(id), is_buy(buy), price(p), quantity(qty), timestamp_ns(ts) {}
    
    Order() : order_id(0), is_buy(true), price(0.0), quantity(0), timestamp_ns(0) {}
};

struct PriceLevel {
    double price;
    uint64_t total_quantity;
    
    PriceLevel(double p = 0.0, uint64_t qty = 0)
        : price(p), total_quantity(qty) {}
};

struct Trade {
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    double price;
    uint64_t quantity;
    uint64_t timestamp_ns;
    
    Trade(uint64_t buy_id, uint64_t sell_id, double p, uint64_t qty, uint64_t ts)
        : buy_order_id(buy_id), sell_order_id(sell_id), price(p), quantity(qty), timestamp_ns(ts) {}
};

