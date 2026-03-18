#pragma once
#include <cassert>
#include <cstdlib>
#include <memory>
#include <new>
#include <utility>

template <typename T>
class RawMemory {
 public:
  RawMemory() = default;

  explicit RawMemory(size_t capacity) : buffer_(Allocate(capacity)), capacity_(capacity) {}

  RawMemory(const RawMemory&) = delete;
  RawMemory(RawMemory&& other) noexcept {
    if (other.capacity_ > 0) {
      std::swap(buffer_, other.buffer_);
      std::swap(capacity_, other.capacity_);
    }
  }
  RawMemory& operator=(const RawMemory& rhs) = delete;
  RawMemory& operator=(RawMemory&& rhs) noexcept {
    if (this != &rhs) {
      std::swap(buffer_, rhs.buffer_);
      std::swap(capacity_, rhs.capacity_);
    }
    return *this;
  }

  ~RawMemory() {
    Deallocate(buffer_);
  }

  T* operator+(size_t offset) noexcept {
    // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
    assert(offset <= capacity_);
    return buffer_ + offset;
  }

  const T* operator+(size_t offset) const noexcept {
    return const_cast<RawMemory&>(*this) + offset;
  }

  const T& operator[](size_t index) const noexcept {
    return const_cast<RawMemory&>(*this)[index];
  }

  T& operator[](size_t index) noexcept {
    assert(index < capacity_);
    return buffer_[index];
  }

  void Swap(RawMemory& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
  }

  const T* GetAddress() const noexcept {
    return buffer_;
  }

  T* GetAddress() noexcept {
    return buffer_;
  }

  size_t Capacity() const {
    return capacity_;
  }

 private:
  // Выделяет сырую память под n элементов и возвращает указатель на неё
  static T* Allocate(size_t n) {
    return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
  }

  // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
  static void Deallocate(T* buf) noexcept {
    operator delete(buf);
  }

  T* buffer_ = nullptr;
  size_t capacity_ = 0;
};

template <typename T>
class Vector {
 public:
  using iterator = T*;
  using const_iterator = const T*;

  Vector() noexcept = default;

  explicit Vector(size_t size) : data_(size), size_(size) {
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
  }

  Vector(const Vector& other) : data_(other.size_), size_(other.size_) {
    std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
  }

  Vector(Vector&& other) noexcept : data_(std::move(other.data_)), size_(other.size_) {
    other.size_ = 0;
  }

  Vector& operator=(const Vector& rhs) {
    if (this != &rhs) {
      if (rhs.size_ > Capacity()) {
        Vector rhs_copy(rhs);
        Swap(rhs_copy);
      } else {
        CopyInPlace(rhs);
      }
    }
    return *this;
  }

  Vector& operator=(Vector&& rhs) noexcept {
    if (this != &rhs) {
      Swap(rhs);
    }
    return *this;
  }

  ~Vector() {
    std::destroy_n(data_.GetAddress(), size_);
  }

  iterator begin() noexcept {
    return data_.GetAddress();
  }
  iterator end() noexcept {
    return data_ + size_;
  }
  const_iterator begin() const noexcept {
    return cbegin();
  }
  const_iterator end() const noexcept {
    return cend();
  }
  const_iterator cbegin() const noexcept {
    return data_.GetAddress();
  }
  const_iterator cend() const noexcept {
    return data_ + size_;
  }

  void Swap(Vector& other) noexcept {
    data_.Swap(other.data_);
    std::swap(size_, other.size_);
  }

  size_t Size() const noexcept {
    return size_;
  }

  size_t Capacity() const noexcept {
    return data_.Capacity();
  }

  const T& operator[](size_t index) const noexcept {
    return const_cast<Vector&>(*this)[index];
  }

  T& operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
  }

  void Reserve(size_t new_capacity) {
    if (new_capacity <= Capacity()) {
      return;
    }

    RawMemory<T> new_data(new_capacity);
    ReserveAt(new_data);
  }

  void Resize(size_t new_size) {
    if (new_size <= size_) {
      std::destroy_n(data_ + new_size, size_ - new_size);
    } else {
      Reserve(new_size);
      std::uninitialized_value_construct_n(data_ + size_, new_size - size_);
    }
    size_ = new_size;
  }

  void PushBack(const T& value) {
    EmplaceBack(value);
  }

  void PushBack(T&& value) {
    EmplaceBack(std::move(value));
  }

  void PopBack() {
    if (size_ == 0) {
      throw std::logic_error("Deletion attempt in an empty vector");
    }
    std::destroy_at(&data_[size_ - 1]);
    --size_;
  }

  template <typename... Args>
  T& EmplaceBack(Args&&... args) {
    if (size_ == Capacity()) {
      RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
      new (new_data + size_) T(std::forward<Args>(args)...);
      ReserveAt(new_data);
    } else {
      new (data_ + size_) T(std::forward<Args>(args)...);
    }
    ++size_;
    return data_[size_ - 1];
  }

  template <typename... Args>
  iterator Emplace(const_iterator pos, Args&&... args) {
    if (pos < begin() || pos > end())
      throw std::out_of_range("Invalid argument");
    size_t idx = pos - begin();
    iterator w_pos = begin() + idx;

    if (size_ == Capacity()) {
      RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
      new (new_data + idx) T(std::forward<Args>(args)...);
      try {
        MoveOrCopyRange(data_.GetAddress(), idx, new_data.GetAddress());
      } catch (...) {
        std::destroy_at(new_data + idx);
        throw;
      }
      try {
        MoveOrCopyRange(data_ + idx, size_ - idx, new_data + idx + 1);
      } catch (...) {
        std::destroy_n(new_data.GetAddress(), idx + 1);
        throw;
      }
      std::destroy_n(data_.GetAddress(), size_);
      data_.Swap(new_data);
      w_pos = begin() + idx;

    } else {
      if (size_ == 0) {
        new (begin()) T(std::forward<Args>(args)...);
      } else {
        T value(std::forward<Args>(args)...);
        MoveOrCopyRange(end() - 1, 1, end());
        std::move_backward(w_pos, end() - 1, end());
        *w_pos = std::move(value);
      }
    }
    ++size_;
    return w_pos;
  }

  iterator Erase(const_iterator pos) /*noexcept(std::is_nothrow_move_assignable_v<T>)*/ {
    if (pos < begin() || pos >= end())
      throw std::out_of_range("Invalid argument");

    iterator w_pos = const_cast<T*>(pos);
    std::move(w_pos + 1, end(), w_pos);
    std::destroy_at(data_ + size_ - 1);
    --size_;
    return w_pos;
  }

  iterator Insert(const_iterator pos, const T& value) {
    return Emplace(pos, value);
  }

  iterator Insert(const_iterator pos, T&& value) {
    return Emplace(pos, std::move(value));
  }

 private:
  void ReserveAt(RawMemory<T>& new_data) {
    MoveOrCopyRange(data_.GetAddress(), size_, new_data.GetAddress());
    std::destroy_n(data_.GetAddress(), size_);
    data_.Swap(new_data);
  }

  void MoveOrCopyRange(T* from, size_t size, T* to) {
    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
      std::uninitialized_move_n(from, size, to);
    } else {
      std::uninitialized_copy_n(from, size, to);
    }
  }

  void CopyInPlace(const Vector& rhs) {
    std::copy(rhs.begin(), rhs.begin() + std::min(rhs.size_, size_), begin());
    if (rhs.size_ < size_) {
      std::destroy_n(data_ + rhs.size_, size_ - rhs.size_);
    } else {
      std::uninitialized_copy_n(rhs.data_ + size_, rhs.size_ - size_, data_ + size_);
    }
    size_ = rhs.size_;
  }

 private:
  RawMemory<T> data_;
  size_t size_ = 0;
};