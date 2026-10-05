#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <new>

struct Pool {
    Pool(std::size_t obj_size, std::size_t capacity)
        : capacity_(capacity) {
        constexpr std::size_t alignment = alignof(std::max_align_t);
        const std::size_t slot_size = obj_size < sizeof(void*) ? sizeof(void*) : obj_size;
        if (slot_size > std::numeric_limits<std::size_t>::max() - (alignment - 1)) {
            throw std::bad_array_new_length();
        }
        stride_ = ((slot_size + alignment - 1) / alignment) * alignment;
        if (capacity_ != 0) {
            if (stride_ > std::numeric_limits<std::size_t>::max() / capacity_) {
                throw std::bad_array_new_length();
            }
            storage_ = ::operator new(stride_ * capacity_);
            auto* bytes = static_cast<unsigned char*>(storage_);
            for (std::size_t i = 0; i < capacity_; ++i) {
                void* next = i + 1 < capacity_ ? bytes + (i + 1) * stride_ : nullptr;
                std::memcpy(bytes + i * stride_, &next, sizeof(next));
            }
            free_head_ = storage_;
        }
    }

    ~Pool() {
        ::operator delete(storage_);
    }

    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;

    void* alloc() {
        if (free_head_ == nullptr) {
            return nullptr;
        }
        void* slot = free_head_;
        std::memcpy(&free_head_, slot, sizeof(free_head_));
        return slot;
    }

    void free(void* pointer) {
        if (pointer == nullptr || storage_ == nullptr) {
            return;
        }
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin = reinterpret_cast<std::uintptr_t>(storage_);
        const std::size_t bytes = stride_ * capacity_;
        if (address < begin || address - begin >= bytes ||
            (address - begin) % stride_ != 0) {
            return;
        }
        std::memcpy(pointer, &free_head_, sizeof(free_head_));
        free_head_ = pointer;
    }

private:
    void* storage_ = nullptr;
    void* free_head_ = nullptr;
    std::size_t stride_ = 0;
    std::size_t capacity_ = 0;
};
