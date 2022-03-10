#ifndef ATOMIC_BUFFER_H
#define ATOMIC_BUFFER_H
#include <stdio.h>
#include <atomic> 
#include <memory>
#include <iostream>

/*JAVIER PRIETO CEPEDA
*SANDRA JUAREZ PUERTA
*/

// Sequential Buffer of generic type T
template <typename T>
class atomic_buffer {
public:
  // Constructs a buffer for n elements
  atomic_buffer(int n);

  // Default destructor
  ~atomic_buffer() = default;

  // Size of buffer
  int size() const noexcept;

  // Is buffer empty?
  bool empty() const noexcept;

  // Is buffer full?
  bool full() const noexcept;

  // Put element x into buffer with marker last.
  //   last=true -> This is the last element of stream
  //   last=false -> This is not the last element of stream
  void put(const T & x, bool last) noexcept;

  // Gets a pair with next element and last indication.
  // Pair is accessed through members first and second
  std::pair<bool,T> get() noexcept;

private:
  // Compute next position after p following circular order
  int next_position(int p) const noexcept;

private:
  // Size of buffer
  const int size_;

  struct item { 
    bool last; 
    T value; 
  };

  // Unique pointer to buffer of size_ elements.
  // NOTE: There are solutions to avoid the extra sizre overhead of
  //       one boole per item.
  std::unique_ptr<item[]> buf_;

  // Next position to read
  std::atomic<int> next_read_ = {0};

  // Next position to write
  std::atomic<int> next_write_ = {0};
  
  //atomic bool
  //std::atomic<bool> gett = {false};
  //std::atomic<bool> putt = {true};  

};

template <typename T>
atomic_buffer<T>::atomic_buffer(int n) :
  size_{n},
  buf_{new item[size_]}
{
}

template <typename T>
int atomic_buffer<T>::size() const noexcept
{
  return size_;
}

template <typename T>
bool atomic_buffer<T>::empty() const noexcept
{
  return next_read_.load(std::memory_order_seq_cst) == next_write_.load(std::memory_order_seq_cst);
}

template <typename T>
bool atomic_buffer<T>::full() const noexcept
{
  const int next = next_position(next_write_.load(std::memory_order_seq_cst));
  return next == next_read_.load(std::memory_order_seq_cst);
}

template <typename T>
void atomic_buffer<T>::put(const T & x, bool last) noexcept
{
  const int next = next_position(next_write_.load(std::memory_order_seq_cst));
  if (full()) {
    while(full());
  }
  
  buf_[next_write_.load(std::memory_order_seq_cst)] = item{last, x};

  //std::cout << "Put: " << buf_[next_write_].value << " in: " << next_write_ << std::endl;

  next_write_.store(next,std::memory_order_seq_cst );
}

template <typename T>
std::pair<bool,T> atomic_buffer<T>::get() noexcept
{
  if (empty()) {
    while(empty());
  }
  auto res = buf_[next_read_.load(std::memory_order_seq_cst)];

  //std::cout << "Get: " << res.value << " at: " << next_read_ << std::endl;

  next_read_.store(next_position(next_read_.load(std::memory_order_seq_cst)),std::memory_order_seq_cst );

  return std::make_pair(res.last, res.value);
}

template <typename T>
int atomic_buffer<T>::next_position(int p) const noexcept
{
  return p + ((p+1>=size_)?(1-size_):1);
}

#endif
