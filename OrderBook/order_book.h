#pragma once

#include "order.h"
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <unordered_map>
#include <memory>

class OrderBook {
public:
    OrderBook(bool enable_matching = false);
    ~OrderBook();
    
    void add_order(const Order& order);
    bool cancel_order(uint64_t order_id);
    bool amend_order(uint64_t order_id, double new_price, uint64_t new_quantity);
    void get_snapshot(size_t depth, std::vector<PriceLevel>& bids, std::vector<PriceLevel>& asks) const;
    void print_book(size_t depth = 10) const;
    
    size_t get_order_count() const { return order_lookup_.size(); }
    const std::vector<Trade>& get_trades() const { return trades_; }
    void clear_trades() { trades_.clear(); }
    
private:
    using OrderIterator = std::list<Order>::iterator;
    
    struct PriceLevelInternal {
        double price;
        uint64_t total_quantity;
        std::list<Order> orders;
        
        PriceLevelInternal(double p) : price(p), total_quantity(0) {}
    };
    
    struct OrderLocation {
        bool is_buy;
        double price;
        OrderIterator order_iter;
    };
    
    bool matching_enabled_;
    std::map<double, PriceLevelInternal, std::greater<double>> bids_;
    std::map<double, PriceLevelInternal, std::less<double>> asks_;
    std::unordered_map<uint64_t, OrderLocation> order_lookup_;
    std::vector<Trade> trades_;
    
    void add_order_internal(const Order& order);
    void match_orders();
    bool can_match() const;
    
    void add_order_to_side(const Order& order, 
                          std::map<double, PriceLevelInternal, std::greater<double>>& side);
    void add_order_to_side(const Order& order, 
                          std::map<double, PriceLevelInternal, std::less<double>>& side);
    
    template<typename Comparator>
    bool cancel_order_from_side(uint64_t order_id, 
                                std::map<double, PriceLevelInternal, Comparator>& side,
                                double price);
    
    template<typename Comparator>
    void get_snapshot_from_side(size_t depth, 
                               const std::map<double, PriceLevelInternal, Comparator>& side,
                               std::vector<PriceLevel>& levels) const;
};

