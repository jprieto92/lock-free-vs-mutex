#ifndef SEQ_BUFFER_H
#define SEQ_BUFFER_H

#include <memory>

struct full_buffer {};
struct empty_buffer {};

// Sequential Buffer of generic type T
template <typename T>
class seq_buffer {
public:
  // Constructs a buffer for n elements
  seq_buffer(int n);

  // Default destructor
  ~seq_buffer() = default;

  // Size of buffer
  int size() const noexcept;

  // Is buffer empty?
  bool empty() const noexcept;

  // Is buffer full?
  bool full() const noexcept;

  // Put element x into buffer with marker last.
  //   last=true -> This is the last element of stream
  //   last=false -> This is not the last element of stream
  void put(const T & x, bool last);

  // Gets a pair wit next element and last indication.
  // Pair is accessed through members first and second
  std::pair<bool,T> get();

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
  int next_read_ = 0;

  // Next position to write
  int next_write_ = 0;
};

template <typename T>
seq_buffer<T>::seq_buffer(int n) :
  size_{n},
  buf_{new item[size_]}
{
}

template <typename T>
int seq_buffer<T>::size() const noexcept
{
  return size_;
}

template <typename T>
bool seq_buffer<T>::empty() const noexcept
{
  return next_read_ == next_write_;
}

template <typename T>
bool seq_buffer<T>::full() const noexcept
{
  const int next = next_position(next_write_);
  return next == next_read_;
}

template <typename T>
void seq_buffer<T>::put(const T & x, bool last)
{
  const int next = next_position(next_write_);
  if (next == next_read_) throw full_buffer{};
  buf_[next_write_] = item{last, x};
  next_write_ = next;
}

template <typename T>
std::pair<bool,T> seq_buffer<T>::get()
{
  if (empty()) throw empty_buffer{};
  auto res = buf_[next_read_];
  next_read_ = next_position(next_read_);
  return std::make_pair(res.last, res.value);
}

template <typename T>
int seq_buffer<T>::next_position(int p) const noexcept
{
  return p + ((p+1>=size_)?(1-size_):1);
}

#endif
