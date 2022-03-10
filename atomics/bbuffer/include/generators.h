#ifndef GENERATORS_H
#define GENERATORS_H

#include <string>
#include <random>
#include <fstream>

#include <iostream>

class random_numeric_generator {
public:
  random_numeric_generator(long long n) noexcept : nitems_{n} {}
  long operator()() noexcept { nitems_--;  return ud_(rd_); }
  bool finished() const noexcept { return nitems_ <=0; }
private:
  long long  nitems_;
  std::random_device rd_ {};
  std::uniform_int_distribution<long> ud_{std::numeric_limits<long>::min(), std::numeric_limits<long>::max()};
};

template <class T>
class file_generator {
public:
  file_generator(const std::string & name) noexcept : ifs{name} {}
  T operator()() noexcept { T x; ifs >> x >> std::ws; return x; }
  bool finished() const noexcept { return !ifs; }
private:
  std::ifstream ifs;
};

#endif
