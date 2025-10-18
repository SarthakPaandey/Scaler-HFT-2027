# Limit Order Book Implementation

High-performance in-memory order book for HFT applications, implemented in C++17.

## Features

- **Core Operations**: Add, Cancel, Amend orders with O(log n) or better complexity
- **Snapshot**: Get aggregated view of top N price levels
- **Memory Pool** (BONUS): Custom allocator to reduce heap fragmentation
- **Matching Engine** (EXTRA CREDIT): Automatic trade execution when prices cross

## Build

### Using Make
```bash
make release
./orderbook
```

### Using CMake
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./orderbook
```

## Structure

- `order.h` - Order, PriceLevel, and Trade structures
- `order_book.h` - OrderBook class interface
- `order_book.cpp` - Implementation
- `memory_pool.h` - Memory pool allocator
- `main.cpp` - Test suite

## Performance

Tested on Apple M1 Pro:
- Add Order: ~0.2 μs
- Cancel Order: ~0.08 μs
- Snapshot: ~0.03 μs

## Design

- Bids: `std::map` with descending order (highest price first)
- Asks: `std::map` with ascending order (lowest price first)
- Orders: `std::list` per price level for FIFO
- Lookup: `std::unordered_map` for O(1) cancel/amend

Matching engine (optional) executes trades when `best_bid >= best_ask`.

## Testing

Run `./orderbook` to execute 8 test scenarios including performance benchmarks.
