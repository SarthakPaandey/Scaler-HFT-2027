#include "order_book.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>

OrderBook::OrderBook(bool enable_matching) : matching_enabled_(enable_matching) {}
OrderBook::~OrderBook() = default;

void OrderBook::add_order(const Order& order) {
    if (matching_enabled_) {
        Order remaining_order = order;
        
        while (remaining_order.quantity > 0) {
            bool can_match_order = false;
            
            if (remaining_order.is_buy && !asks_.empty()) {
                can_match_order = remaining_order.price >= asks_.begin()->first;
            } else if (!remaining_order.is_buy && !bids_.empty()) {
                can_match_order = remaining_order.price <= bids_.begin()->first;
            }
            
            if (!can_match_order) {
                break;
            }
            
            auto& contra_level = remaining_order.is_buy ? 
                                asks_.begin()->second : 
                                bids_.begin()->second;
            auto& contra_order = contra_level.orders.front();
            
            uint64_t trade_qty = std::min(remaining_order.quantity, contra_order.quantity);
            double trade_price = contra_level.price;
            
            auto now = std::chrono::high_resolution_clock::now();
            auto ts = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
            
            if (remaining_order.is_buy) {
                trades_.emplace_back(remaining_order.order_id, contra_order.order_id, 
                                    trade_price, trade_qty, ts);
            } else {
                trades_.emplace_back(contra_order.order_id, remaining_order.order_id, 
                                    trade_price, trade_qty, ts);
            }
            
            remaining_order.quantity -= trade_qty;
            contra_order.quantity -= trade_qty;
            contra_level.total_quantity -= trade_qty;
            
            if (contra_order.quantity == 0) {
                uint64_t contra_id = contra_order.order_id;
                contra_level.orders.pop_front();
                order_lookup_.erase(contra_id);
                
                if (contra_level.orders.empty()) {
                    if (remaining_order.is_buy) {
                        asks_.erase(asks_.begin());
                    } else {
                        bids_.erase(bids_.begin());
                    }
                }
            }
        }
        
        if (remaining_order.quantity > 0) {
            add_order_internal(remaining_order);
        }
    } else {
        add_order_internal(order);
    }
}

void OrderBook::add_order_internal(const Order& order) {
    if (order.is_buy) {
        add_order_to_side(order, bids_);
    } else {
        add_order_to_side(order, asks_);
    }
}

bool OrderBook::can_match() const {
    if (bids_.empty() || asks_.empty()) {
        return false;
    }
    return bids_.begin()->first >= asks_.begin()->first;
}

void OrderBook::add_order_to_side(const Order& order, 
                                  std::map<double, PriceLevelInternal, std::greater<double>>& side) {
    auto it = side.find(order.price);
    if (it == side.end()) {
        it = side.emplace(order.price, PriceLevelInternal(order.price)).first;
    }
    
    it->second.orders.push_back(order);
    it->second.total_quantity += order.quantity;
    
    OrderLocation loc;
    loc.is_buy = order.is_buy;
    loc.price = order.price;
    loc.order_iter = std::prev(it->second.orders.end());
    
    order_lookup_[order.order_id] = loc;
}

void OrderBook::add_order_to_side(const Order& order, 
                                  std::map<double, PriceLevelInternal, std::less<double>>& side) {
    auto it = side.find(order.price);
    if (it == side.end()) {
        it = side.emplace(order.price, PriceLevelInternal(order.price)).first;
    }
    
    it->second.orders.push_back(order);
    it->second.total_quantity += order.quantity;
    
    OrderLocation loc;
    loc.is_buy = order.is_buy;
    loc.price = order.price;
    loc.order_iter = std::prev(it->second.orders.end());
    
    order_lookup_[order.order_id] = loc;
}

bool OrderBook::cancel_order(uint64_t order_id) {
    auto lookup_it = order_lookup_.find(order_id);
    if (lookup_it == order_lookup_.end()) {
        return false;
    }
    
    const OrderLocation& loc = lookup_it->second;
    bool success = false;
    
    if (loc.is_buy) {
        success = cancel_order_from_side(order_id, bids_, loc.price);
    } else {
        success = cancel_order_from_side(order_id, asks_, loc.price);
    }
    
    if (success) {
        order_lookup_.erase(lookup_it);
    }
    
    return success;
}

template<typename Comparator>
bool OrderBook::cancel_order_from_side(uint64_t order_id,
                                      std::map<double, PriceLevelInternal, Comparator>& side,
                                      double price) {
    auto price_it = side.find(price);
    if (price_it == side.end()) {
        return false;
    }
    
    auto& price_level = price_it->second;
    auto lookup_it = order_lookup_.find(order_id);
    
    if (lookup_it == order_lookup_.end()) {
        return false;
    }
    
    OrderIterator order_iter = lookup_it->second.order_iter;
    
    price_level.total_quantity -= order_iter->quantity;
    price_level.orders.erase(order_iter);
    
    if (price_level.orders.empty()) {
        side.erase(price_it);
    }
    
    return true;
}

bool OrderBook::amend_order(uint64_t order_id, double new_price, uint64_t new_quantity) {
    auto lookup_it = order_lookup_.find(order_id);
    if (lookup_it == order_lookup_.end()) {
        return false;
    }
    
    const OrderLocation& loc = lookup_it->second;
    OrderIterator order_iter = loc.order_iter;
    
    if (new_price != loc.price) {
        Order new_order = *order_iter;
        new_order.price = new_price;
        new_order.quantity = new_quantity;
        
        cancel_order(order_id);
        add_order(new_order);
        return true;
    }
    
    if (loc.is_buy) {
        auto price_it = bids_.find(loc.price);
        if (price_it == bids_.end()) {
            return false;
        }
        
        auto& price_level = price_it->second;
        price_level.total_quantity -= order_iter->quantity;
        price_level.total_quantity += new_quantity;
        order_iter->quantity = new_quantity;
    } else {
        auto price_it = asks_.find(loc.price);
        if (price_it == asks_.end()) {
            return false;
        }
        
        auto& price_level = price_it->second;
        price_level.total_quantity -= order_iter->quantity;
        price_level.total_quantity += new_quantity;
        order_iter->quantity = new_quantity;
    }
    
    return true;
}

void OrderBook::get_snapshot(size_t depth, 
                            std::vector<PriceLevel>& bids, 
                            std::vector<PriceLevel>& asks) const {
    bids.clear();
    asks.clear();
    
    get_snapshot_from_side(depth, bids_, bids);
    get_snapshot_from_side(depth, asks_, asks);
}

template<typename Comparator>
void OrderBook::get_snapshot_from_side(size_t depth,
                                      const std::map<double, PriceLevelInternal, Comparator>& side,
                                      std::vector<PriceLevel>& levels) const {
    levels.reserve(std::min(depth, side.size()));
    
    size_t count = 0;
    for (const auto& [price, level] : side) {
        if (count >= depth) break;
        levels.emplace_back(level.price, level.total_quantity);
        ++count;
    }
}

void OrderBook::print_book(size_t depth) const {
    std::vector<PriceLevel> bids, asks;
    get_snapshot(depth, bids, asks);
    
    std::cout << "\n========== ORDER BOOK ==========\n";
    std::cout << std::fixed << std::setprecision(2);
    
    std::cout << std::setw(15) << "BID QTY" << " | "
              << std::setw(10) << "BID PRICE" << " | "
              << std::setw(10) << "ASK PRICE" << " | "
              << std::setw(15) << "ASK QTY" << "\n";
    std::cout << std::string(70, '-') << "\n";
    
    size_t max_levels = std::max(bids.size(), asks.size());
    for (size_t i = 0; i < max_levels; ++i) {
        if (i < bids.size()) {
            std::cout << std::setw(15) << bids[i].total_quantity << " | "
                      << std::setw(10) << bids[i].price << " | ";
        } else {
            std::cout << std::setw(15) << "" << " | "
                      << std::setw(10) << "" << " | ";
        }
        
        if (i < asks.size()) {
            std::cout << std::setw(10) << asks[i].price << " | "
                      << std::setw(15) << asks[i].total_quantity << "\n";
        } else {
            std::cout << std::setw(10) << "" << " | "
                      << std::setw(15) << "" << "\n";
        }
    }
    
    std::cout << "================================\n";
    std::cout << "Total Orders: " << order_lookup_.size() << "\n\n";
}

