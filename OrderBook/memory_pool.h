#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>

template<typename T, size_t BlockSize = 4096>
class MemoryPool {
public:
    MemoryPool() {
        allocate_block();
    }
    
    ~MemoryPool() {}
    
    T* allocate() {
        if (free_slots_.empty()) {
            allocate_block();
        }
        
        T* slot = free_slots_.back();
        free_slots_.pop_back();
        return slot;
    }
    
    void deallocate(T* ptr) {
        if (ptr == nullptr) return;
        ptr->~T();
        free_slots_.push_back(ptr);
    }
    
    template<typename... Args>
    T* construct(Args&&... args) {
        T* slot = allocate();
        new (slot) T(std::forward<Args>(args)...);
        return slot;
    }
    
    void destroy(T* ptr) {
        deallocate(ptr);
    }
    
    size_t total_allocated() const { return blocks_.size() * BlockSize; }
    size_t available() const { return free_slots_.size(); }
    size_t in_use() const { return total_allocated() - available(); }
    
private:
    struct Block {
        alignas(T) uint8_t data[BlockSize * sizeof(T)];
    };
    
    std::vector<std::unique_ptr<Block>> blocks_;
    std::vector<T*> free_slots_;
    
    void allocate_block() {
        auto block = std::make_unique<Block>();
        T* block_start = reinterpret_cast<T*>(block->data);
        
        for (size_t i = 0; i < BlockSize; ++i) {
            free_slots_.push_back(block_start + i);
        }
        
        blocks_.push_back(std::move(block));
    }
    
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
};

template<typename T, size_t BlockSize = 4096>
class PoolAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    
    template<typename U>
    struct rebind {
        using other = PoolAllocator<U, BlockSize>;
    };
    
    PoolAllocator() : pool_(std::make_shared<MemoryPool<T, BlockSize>>()) {}
    
    PoolAllocator(const PoolAllocator& other) : pool_(other.pool_) {}
    
    template<typename U>
    PoolAllocator(const PoolAllocator<U, BlockSize>& other) 
        : pool_(std::reinterpret_pointer_cast<MemoryPool<T, BlockSize>>(other.pool_)) {}
    
    T* allocate(size_t n) {
        if (n != 1) {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }
        return pool_->allocate();
    }
    
    void deallocate(T* ptr, size_t n) {
        if (n != 1) {
            ::operator delete(ptr);
        } else {
            pool_->deallocate(ptr);
        }
    }
    
    template<typename U>
    bool operator==(const PoolAllocator<U, BlockSize>& other) const {
        return pool_ == other.pool_;
    }
    
    template<typename U>
    bool operator!=(const PoolAllocator<U, BlockSize>& other) const {
        return !(*this == other);
    }
    
private:
    template<typename U, size_t BS>
    friend class PoolAllocator;
    
    std::shared_ptr<MemoryPool<T, BlockSize>> pool_;
};

