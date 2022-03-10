#ifndef LOCKED_BUFFER_H
#define LOCKED_BUFFER_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>

/*JAVIER PRIETO CEPEDA
*SANDRA JUAREZ PUERTA
*/

// Locked buffer of generic type T
template <typename T>
class locked_buffer {
public:
  // Constructs a buffer for n elements
  locked_buffer(int n);

  // Default destructor
  ~locked_buffer() = default;

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

  // Gets a pair wit next element and last indication.
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
  std::mutex m; /*mutex variable*/
  std::condition_variable full_cv; /*condition variable that determine if buffer is full*/
  std::condition_variable empty_cv;  /*condition variable that determine if buffer is empty*/
  // Next position to read
  int next_read_ = 0;

  // Next position to write
  int next_write_ = 0;
};

template <typename T>
locked_buffer<T>::locked_buffer(int n) :
  size_{n},
  buf_{new item[size_]}
{
}

template <typename T>
int locked_buffer<T>::size() const noexcept
{
  return size_;
}

template <typename T>
bool locked_buffer<T>::empty() const noexcept
{
  return next_read_ == next_write_;
}

template <typename T>
bool locked_buffer<T>::full() const noexcept
{
  const int next = next_position(next_write_);
  return next == next_read_;
}

template <typename T>
void locked_buffer<T>::put(const T & x, bool last) noexcept
{
  const int next = next_position(next_write_);
  
  std::unique_lock<std::mutex> lk(m); 
 
  if (next == next_read_) {
    //throw full_buffer{};
    full_cv.wait(lk);
  }
  buf_[next_write_] = item{last, x};
  //std::cout << "Put: " << buf_[next_write_].value << " in: " << next_write_ << std::endl;
  next_write_ = next;
  //std::cout << "Put: " << x << " in: " << next_write_ << std::endl;
  lk.unlock();
  empty_cv.notify_one();
}

template <typename T>
std::pair<bool,T> locked_buffer<T>::get() noexcept
{
  std::unique_lock<std::mutex> lk(m);

  if (next_read_ == next_write_) {
    //throw empty_buffer{};
    empty_cv.wait(lk);
  }
  auto res = buf_[next_read_];
  next_read_ = next_position(next_read_);
  
  //std::cout << "Get: " << res.value << " at: " << next_read_ << std::endl;
  lk.unlock();
  full_cv.notify_one();

  return std::make_pair(res.last, res.value);
}

template <typename T>
int locked_buffer<T>::next_position(int p) const noexcept
{
  return p + ((p+1>=size_)?(1-size_):1);
}

#endif
