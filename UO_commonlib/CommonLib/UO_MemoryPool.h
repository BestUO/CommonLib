/*-
 * Copyright (c) 2013 Cosku Acay, http://www.coskuacay.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MEMORY_POOL_H_UO
#define MEMORY_POOL_H_UO

//#include <climits>
//#include <cstddef>
#include <iostream>
#include <stdint.h>
#include "UO_Public.h"

template <typename T, size_t BlockSize = 4096>
class MemoryPool:public UO_SpinLock
{
  public:
    /* Member types */
    typedef T               value_type;
    typedef T*              pointer;
    typedef T&              reference;
    typedef const T*        const_pointer;
    typedef const T&        const_reference;
    typedef size_t          size_type;
    typedef ptrdiff_t       difference_type;
    //typedef std::false_type propagate_on_container_copy_assignment;
    //typedef std::true_type  propagate_on_container_move_assignment;
    //typedef std::true_type  propagate_on_container_swap;

    template <typename U> struct rebind {
      typedef MemoryPool<U> other;
    };

    /* Member functions */
    MemoryPool() noexcept
    {
        currentBlock_ = nullptr;
        currentSlot_ = nullptr;
        lastSlot_ = nullptr;
        freeSlots_ = nullptr;
    }
    explicit MemoryPool(const MemoryPool& memoryPool) noexcept : MemoryPool()
    {}
    explicit MemoryPool(MemoryPool&& memoryPool) noexcept
    {  
        currentBlock_ = memoryPool.currentBlock_;
        memoryPool.currentBlock_ = nullptr;
        currentSlot_ = memoryPool.currentSlot_;
        lastSlot_ = memoryPool.lastSlot_;
        freeSlots_ = memoryPool.freeSlots;
    }
    template <class U> explicit MemoryPool(const MemoryPool<U>& memoryPool) noexcept : MemoryPool()
    {}

    ~MemoryPool() noexcept {freeMemoryPool();}

    void freeMemoryPool() noexcept
    {
        Lock();
        slot_pointer_ curr = currentBlock_;
        if(!curr)
            return;
        while (curr != nullptr) 
        {
            slot_pointer_ prev = curr->next;
            operator delete(reinterpret_cast<void*>(curr));
            curr = prev;
        }
        currentBlock_ = nullptr;
        freeSlots_ = nullptr;
        UnLock();
    }

    MemoryPool& operator=(const MemoryPool& memoryPool) = delete;
    MemoryPool& operator=(MemoryPool&& memoryPool) noexcept
    {
        if (this != &memoryPool)
        {
            std::swap(currentBlock_, memoryPool.currentBlock_);
            currentSlot_ = memoryPool.currentSlot_;
            lastSlot_ = memoryPool.lastSlot_;
            freeSlots_ = memoryPool.freeSlots;
        }
        return *this;
    }

    pointer address(reference x) const noexcept {return &x;}
    const_pointer address(const_reference x) const noexcept {return &x;}

    // Can only allocate one object at a time. n and hint are ignored
    pointer allocate(size_type n = 1, const_pointer hint = 0)
    {
        Lock();
        pointer result = nullptr;
        if(freeSlots_ != nullptr) 
        {
            result = reinterpret_cast<pointer>(freeSlots_);
            freeSlots_ = freeSlots_->next;
        }
        else 
        {
            if(currentSlot_ >= lastSlot_)
                allocateBlock();
            result = reinterpret_cast<pointer>(currentSlot_++);
        }
        UnLock();
        return result;
    }
    void deallocate(pointer p, size_type n = 1)
    {
        Lock();
        if (p != nullptr) 
        {
            reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;
            freeSlots_ = reinterpret_cast<slot_pointer_>(p);
        }
        UnLock();
    }

    size_type max_size() const noexcept
    {
        size_type maxBlocks = -1 / BlockSize;
        return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;  
    }

    template <class U, class... Args> void construct(U* p, Args&&... args) {new (p) U(std::forward<Args>(args)...);}
    template <class U> void destroy(U* p) {p->~U();}

    template <class... Args> pointer newElement(Args&&... args)
    {
        pointer result = allocate();
        construct<value_type>(result, std::forward<Args>(args)...);
        return result;
    }
    void deleteElement(pointer p)
    {
        if (p != nullptr) 
        {
            p->~value_type();
            deallocate(p);
        }
    }

  private:
    union Slot_ 
    {
      value_type element;
      Slot_* next;
    };

    typedef char* data_pointer_;
    typedef Slot_ slot_type_;
    typedef Slot_ *slot_pointer_;

    slot_pointer_ currentBlock_;
    slot_pointer_ currentSlot_;
    slot_pointer_ lastSlot_;
    slot_pointer_ freeSlots_;

    size_type padPointer(data_pointer_ p, size_type align) const noexcept
    {
        uintptr_t result = reinterpret_cast<uintptr_t>(p);
        return ((align - result) % align);
    }

    void allocateBlock()
    {
          // Allocate space for the new block and store a pointer to the previous one
        data_pointer_ newBlock = reinterpret_cast<data_pointer_>(operator new(BlockSize));
        reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;
        currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);
        // Pad block body to staisfy the alignment requirements for elements
        data_pointer_ body = newBlock + sizeof(slot_pointer_);
        size_type bodyPadding = padPointer(body, alignof(slot_type_));
        currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);
        lastSlot_ = reinterpret_cast<slot_pointer_>(newBlock + BlockSize - sizeof(slot_type_) + 1);
    }

    static_assert(BlockSize >= 2 * sizeof(slot_type_), "BlockSize too small.");
};

#endif // MEMORY_POOL_H
