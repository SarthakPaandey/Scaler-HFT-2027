#include "order_book.h"
#include "memory_pool.h"
#include <iostream>
#include <chrono>
#include <random>
#include <iomanip>

uint64_t get_timestamp_ns() {
    auto now = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
    return ns.count();
}

void test_basic_operations() {
    std::cout << "\n=== TEST 1: Basic Operations ===\n";
    
    OrderBook book;
    
    book.add_order(Order(1, true, 100.00, 50, get_timestamp_ns()));
    book.add_order(Order(2, true, 100.00, 30, get_timestamp_ns()));
    book.add_order(Order(3, true, 99.50, 100, get_timestamp_ns()));
    book.add_order(Order(4, true, 99.00, 75, get_timestamp_ns()));
    
    book.add_order(Order(5, false, 100.50, 40, get_timestamp_ns()));
    book.add_order(Order(6, false, 101.00, 60, get_timestamp_ns()));
    book.add_order(Order(7, false, 101.50, 80, get_timestamp_ns()));
    
    book.print_book(10);
}

void test_cancel_operations() {
    std::cout << "\n=== TEST 2: Cancel Operations ===\n";
    
    OrderBook book;
    
    book.add_order(Order(1, true, 100.00, 50, get_timestamp_ns()));
    book.add_order(Order(2, true, 100.00, 30, get_timestamp_ns()));
    book.add_order(Order(3, true, 99.50, 100, get_timestamp_ns()));
    book.add_order(Order(4, false, 100.50, 40, get_timestamp_ns()));
    book.add_order(Order(5, false, 101.00, 60, get_timestamp_ns()));
    
    std::cout << "Before cancellation:\n";
    book.print_book(5);
    
    bool result = book.cancel_order(1);
    std::cout << "Cancel order 1: " << (result ? "SUCCESS" : "FAILED") << "\n";
    
    std::cout << "\nAfter cancellation:\n";
    book.print_book(5);
    
    result = book.cancel_order(999);
    std::cout << "Cancel order 999: " << (result ? "SUCCESS" : "FAILED") << " (expected FAILED)\n";
}

void test_amend_operations() {
    std::cout << "\n=== TEST 3: Amend Operations ===\n";
    
    OrderBook book;
    
    book.add_order(Order(1, true, 100.00, 50, get_timestamp_ns()));
    book.add_order(Order(2, true, 99.50, 100, get_timestamp_ns()));
    book.add_order(Order(3, false, 100.50, 40, get_timestamp_ns()));
    
    std::cout << "Before amendment:\n";
    book.print_book(5);
    
    bool result = book.amend_order(1, 100.00, 100);
    std::cout << "\nAmend order 1 quantity to 100: " << (result ? "SUCCESS" : "FAILED") << "\n";
    
    std::cout << "\nAfter quantity amendment:\n";
    book.print_book(5);
    
    result = book.amend_order(1, 99.00, 100);
    std::cout << "\nAmend order 1 price to 99.00: " << (result ? "SUCCESS" : "FAILED") << "\n";
    
    std::cout << "\nAfter price amendment:\n";
    book.print_book(5);
}

void test_snapshot() {
    std::cout << "\n=== TEST 4: Snapshot Functionality ===\n";
    
    OrderBook book;
    
    for (int i = 0; i < 10; ++i) {
        book.add_order(Order(i, true, 100.0 - i * 0.5, 50 + i * 10, get_timestamp_ns()));
    }
    
    for (int i = 10; i < 20; ++i) {
        book.add_order(Order(i, false, 101.0 + (i - 10) * 0.5, 50 + i * 10, get_timestamp_ns()));
    }
    
    std::vector<PriceLevel> bids, asks;
    book.get_snapshot(5, bids, asks);
    
    std::cout << "Top 5 Bids:\n";
    for (const auto& level : bids) {
        std::cout << "  Price: " << std::fixed << std::setprecision(2) 
                  << level.price << ", Quantity: " << level.total_quantity << "\n";
    }
    
    std::cout << "\nTop 5 Asks:\n";
    for (const auto& level : asks) {
        std::cout << "  Price: " << std::fixed << std::setprecision(2) 
                  << level.price << ", Quantity: " << level.total_quantity << "\n";
    }
}

void test_performance() {
    std::cout << "\n=== TEST 6: Performance Benchmark ===\n";
    
    OrderBook book;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> price_dist(9500, 10500);
    std::uniform_int_distribution<> qty_dist(10, 1000);
    std::uniform_int_distribution<> side_dist(0, 1);
    
    const int NUM_ORDERS = 10000;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ORDERS; ++i) {
        double price = price_dist(gen) / 100.0;
        uint64_t qty = qty_dist(gen);
        bool is_buy = side_dist(gen) == 1;
        
        book.add_order(Order(i, is_buy, price, qty, get_timestamp_ns()));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Added " << NUM_ORDERS << " orders in " 
              << duration.count() << " μs\n";
    std::cout << "Average time per add: " 
              << (double)duration.count() / NUM_ORDERS << " μs\n";
    
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ORDERS / 2; ++i) {
        book.cancel_order(i * 2);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "\nCancelled " << NUM_ORDERS / 2 << " orders in " 
              << duration.count() << " μs\n";
    std::cout << "Average time per cancel: " 
              << (double)duration.count() / (NUM_ORDERS / 2) << " μs\n";
    
    std::vector<PriceLevel> bids, asks;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        book.get_snapshot(10, bids, asks);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "\n1000 snapshots (depth=10) in " 
              << duration.count() << " μs\n";
    std::cout << "Average time per snapshot: " 
              << (double)duration.count() / 1000 << " μs\n";
    
    std::cout << "\nFinal order count: " << book.get_order_count() << "\n";
}

void test_fifo_ordering() {
    std::cout << "\n=== TEST 5: FIFO Ordering ===\n";
    
    OrderBook book;
    
    book.add_order(Order(1, true, 100.00, 50, get_timestamp_ns()));
    book.add_order(Order(2, true, 100.00, 30, get_timestamp_ns()));
    book.add_order(Order(3, true, 100.00, 70, get_timestamp_ns()));
    
    std::cout << "Added 3 orders at price 100.00 (IDs: 1, 2, 3)\n";
    std::cout << "Total quantity should be 150 (50 + 30 + 70)\n\n";
    
    book.print_book(5);
    
    book.cancel_order(2);
    std::cout << "\nAfter cancelling order 2:\n";
    std::cout << "Total quantity should be 120 (50 + 70)\n\n";
    
    book.print_book(5);
}

void test_matching_engine() {
    std::cout << "\n=== TEST 7: Matching Engine (EXTRA CREDIT) ===\n";
    
    OrderBook book(true);
    
    std::cout << "Adding sell orders first:\n";
    book.add_order(Order(1, false, 100.50, 50, get_timestamp_ns()));
    book.add_order(Order(2, false, 101.00, 30, get_timestamp_ns()));
    book.add_order(Order(3, false, 101.50, 40, get_timestamp_ns()));
    book.print_book(5);
    
    std::cout << "\nAdding buy order at 100.00 (no match):\n";
    book.add_order(Order(4, true, 100.00, 60, get_timestamp_ns()));
    book.print_book(5);
    
    std::cout << "\nAdding buy order at 101.00 (should match with sell order 1 and 2):\n";
    book.add_order(Order(5, true, 101.00, 70, get_timestamp_ns()));
    book.print_book(5);
    
    std::cout << "\nTrades executed:\n";
    const auto& trades = book.get_trades();
    for (const auto& trade : trades) {
        std::cout << "  Buy Order " << trade.buy_order_id 
                  << " x Sell Order " << trade.sell_order_id
                  << " @ " << std::fixed << std::setprecision(2) << trade.price 
                  << " qty: " << trade.quantity << "\n";
    }
    std::cout << "Total trades: " << trades.size() << "\n";
    
    std::cout << "\nAdding aggressive sell order that crosses the spread:\n";
    book.add_order(Order(6, false, 99.50, 100, get_timestamp_ns()));
    book.print_book(5);
    
    std::cout << "\nAll trades:\n";
    for (const auto& trade : book.get_trades()) {
        std::cout << "  Buy " << trade.buy_order_id 
                  << " x Sell " << trade.sell_order_id
                  << " @ " << std::fixed << std::setprecision(2) << trade.price 
                  << " qty: " << trade.quantity << "\n";
    }
}

void test_memory_pool() {
    std::cout << "\n=== TEST 8: Memory Pool Demonstration ===\n";
    
    MemoryPool<Order, 1024> pool;
    
    std::cout << "Memory pool initialized\n";
    std::cout << "Total allocated slots: " << pool.total_allocated() << "\n";
    std::cout << "Available slots: " << pool.available() << "\n";
    
    std::vector<Order*> orders;
    const int NUM_ALLOCS = 100;
    
    for (int i = 0; i < NUM_ALLOCS; ++i) {
        Order* order = pool.construct(i, true, 100.0 + i, 100, get_timestamp_ns());
        orders.push_back(order);
    }
    
    std::cout << "\nAfter allocating " << NUM_ALLOCS << " orders:\n";
    std::cout << "Available slots: " << pool.available() << "\n";
    std::cout << "In use: " << pool.in_use() << "\n";
    
    for (int i = 0; i < NUM_ALLOCS / 2; ++i) {
        pool.destroy(orders[i]);
    }
    
    std::cout << "\nAfter deallocating " << NUM_ALLOCS / 2 << " orders:\n";
    std::cout << "Available slots: " << pool.available() << "\n";
    std::cout << "In use: " << pool.in_use() << "\n";
}

int main() {
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     HIGH-PERFORMANCE LIMIT ORDER BOOK IMPLEMENTATION       ║\n";
    std::cout << "║                    C++17 Edition                           ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    try {
        test_basic_operations();
        test_cancel_operations();
        test_amend_operations();
        test_snapshot();
        test_fifo_ordering();
        test_performance();
        test_matching_engine();
        test_memory_pool();
        
        std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
        std::cout << "║              ALL TESTS COMPLETED SUCCESSFULLY              ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════╝\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "\nERROR: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

