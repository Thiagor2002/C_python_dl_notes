#include <algorithm>  // std::copy
#include <iostream>   // std::cout/endl
#include <iterator>   // std::ostream_iterator
#include <memory>     // std::allocator
#include <new>        // placement new
#include <utility>    // std::swap
#include <stddef.h>   // size_t

template <typename T, typename Allocator = std::allocator<T>>
class Vector : Allocator {
public:
    Vector() = default;
    explicit Vector(const Allocator& alloc) : Allocator(alloc) {}
    Vector(size_t n, const T& value = T(),
           const Allocator& alloc = Allocator())
        : Allocator(alloc)
    {
        begin_ = Allocator::allocate(n);
        end_cap_ = begin_ + n;
        try {
            for (end_ = begin_; end_ != end_cap_; ++end_) {
                new (end_) T(value);
            }
        }
        catch (...) {
            destroy(begin_, end_);
            Allocator::deallocate(begin_, n);
            throw;
        }
    }
    Vector(const Vector& rhs) : Allocator(rhs)
    {
        size_t size = rhs.size();
        if (size != 0) {
            begin_ = Allocator::allocate(size);
            try {
                copy(rhs.begin_, rhs.end_, begin_);
            }
            catch (...) {
                Allocator::deallocate(begin_, size);
                throw;
            }
            end_cap_ = end_ = begin_ + size;
        }
    }
    Vector(Vector&& rhs) : Allocator(std::move(rhs))
    {
        begin_ = rhs.begin_;
        end_ = rhs.end_;
        end_cap_ = rhs.end_cap_;
        rhs.begin_ = rhs.end_ = rhs.end_cap_ = nullptr;
    }
    Vector& operator=(const Vector& rhs)
    {
        Vector(rhs).swap(*this);
        return *this;
    }
    Vector& operator=(Vector&& rhs)
    {
        Vector(std::move(rhs)).swap(*this);
        return *this;
    }
    ~Vector()
    {
        if (begin_ != nullptr) {
            destroy(begin_, end_);
            Allocator::deallocate(begin_, end_cap_ - begin_);
        }
    }

    void swap(Vector& rhs)
    {
        using std::swap;
        swap(begin_, rhs.begin_);
        swap(end_, rhs.end_);
        swap(end_cap_, rhs.end_cap_);
        swap(static_cast<Allocator&>(*this), static_cast<Allocator&>(rhs));
    }

    T* data()
    {
        return begin_;
    }
    const T* data() const
    {
        return begin_;
    }
    T* begin()
    {
        return begin_;
    }
    const T* begin() const
    {
        return begin_;
    }
    T* end()
    {
        return end_;
    }
    const T* end() const
    {
        return end_;
    }
    bool empty() const
    {
        return end_ == begin_;
    }
    size_t size() const
    {
        return end_ - begin_;
    }
    void push_back(const T& item)
    {
        if (need_alloc()) {
            size_t old_size = size();
            size_t new_capacity = old_size == 0 ? 1 : old_size * 2;
            T* new_begin = Allocator::allocate(new_capacity);
            T* new_end = new_begin + old_size;
            try {
                new (new_end) T(item);
                try {
                    copy(begin_, end_, new_begin);
                }
                catch (...) {
                    new_end->~T();
                    throw;
                }
            }
            catch (...) {
                Allocator::deallocate(new_begin, new_capacity);
                throw;
            }
            ++new_end;
            destroy(begin_, end_);
            Allocator::deallocate(begin_, end_cap_ - begin_);
            begin_ = new_begin;
            end_ = new_end;
            end_cap_ = new_begin + new_capacity;
        } else {
            new (end_) T(item);
            ++end_;
        }
    }

private:
    bool need_alloc() const
    {
        return end_ == end_cap_;
    }
    static void copy(T* begin, T* end, T* target)
    {
        T* ptr = target;
        try {
            while (begin != end) {
                new (ptr) T(*begin);
                ++begin;
                ++ptr;
            }
        }
        catch (...) {
            destroy(target, ptr);
            throw;
        }
    }
    static void destroy(T* begin, T* end)
    {
        for (T* ptr = begin; ptr != end; ++ptr) {
            ptr->~T();
        }
    }

    T* begin_{};
    T* end_{};
    T* end_cap_{};
};

class Obj {
public:
    Obj(int n) : value_(n) {}
    Obj(const Obj& other) : value_(other.value_)
    {
        if (value_ == 13) {
            throw "Bad";
        }
    }
    friend std::ostream& operator<<(std::ostream& os, const Obj& obj)
    {
        os << obj.value_;
        return os;
    }

private:
    int value_;
};

template <typename T>
void PrintVector(const Vector<T>& v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " "));
    std::cout << "(size " << v.size() << ")" << std::endl;
}

#define PRINT_VECTOR(v) std::cout << #v << ": "; PrintVector(v);

int main()
{
    Vector<Obj> v1(5, 1);
    v1.push_back(2);
    v1.push_back(3);
    try {
        v1.push_back(13);
    }
    catch (...) {
    }
    PRINT_VECTOR(v1);

    Vector<Obj> v2;
    v2 = v1;
    std::cout << "After copy assignment\n";
    PRINT_VECTOR(v1);
    PRINT_VECTOR(v2);

    Vector<Obj> v3;
    v3 = std::move(v1);
    std::cout << "After move assignment\n";
    PRINT_VECTOR(v1);
    PRINT_VECTOR(v3);
}
