#pragma once
#include <atomic>
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>
#include <assert.h>

template <typename T>
class CircularQueueMPMC {
    
    struct Node {
        std::unique_ptr<T> data;
        std::atomic<bool> flag { false };
    };

private:
    std::vector<Node> buffer_;
    size_t Capacity;
    alignas(64) std::atomic<size_t> head_ { 0 };
    alignas(64) std::atomic<size_t> tail_ { 0 };

public:
    CircularQueueMPMC(size_t Capacity)
        : buffer_(Capacity)
    {
        assert(("环形队列的大小应为2的幂",(Capacity & (Capacity - 1)) == 0));
    }

    bool enqueue(std::unique_ptr<T>& data)
    {
        size_t current_tail;
        size_t next;
        do {
            current_tail = tail_.load(std::memory_order_relaxed);
            next = (current_tail + 1) & (Capacity - 1);
            if (next == head_.load(std::memory_order_acquire)) {
                return false;
            }
        } while (!tail_.compare_exchange_weak(current_tail, next, std::memory_order_release));
        buffer_[current_tail].data = std::move(data);
        buffer_[current_tail].flag.store(true, std::memory_order_release);
        return true;
    }

    bool enqueue(std::unique_ptr<T>&& data)
    {
        size_t current_tail;
        size_t next;
        do {
            current_tail = tail_.load(std::memory_order_relaxed);
            next = (current_tail + 1) & (Capacity - 1);
            if (next == head_.load(std::memory_order_acquire)) {
                return false;
            }
        } while (!tail_.compare_exchange_weak(current_tail, next, std::memory_order_release));
        buffer_[current_tail].data = std::move(data);
        buffer_[current_tail].flag.store(true, std::memory_order_release);
        return true;
    }

    bool dequeue(std::unique_ptr<T>& data)
    {
        if (data != nullptr)
            return false;
        size_t current_head;
        size_t next;
        do {
            current_head = head_.load(std::memory_order_relaxed);
            if (current_head == tail_.load(std::memory_order_acquire)) {
                std::cout << "buffer is empty" << std::endl;
                return false;
            }
            if (buffer_[current_head].flag.load(std::memory_order_acquire) == false)
                return false;
            next = (current_head + 1) & (Capacity - 1);
        } while (!head_.compare_exchange_weak(current_head, next, std::memory_order_release));
        data = std::move(buffer_[current_head].data);
        buffer_[current_head].flag.store(false, std::memory_order_release);
        return true;
    }

    bool isEmpty()
    {
        if (head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire))
            return true;
        else
            return false;
    }
};