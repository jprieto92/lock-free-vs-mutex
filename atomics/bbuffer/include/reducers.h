#ifndef REDUCERS_H
#define REDUCERS_H

#include <limits>
#include <string>

class numeric_reducer {
public:
  numeric_reducer() noexcept {}
  void operator()(long x) noexcept {
    min_ = std::min(min_, x);
    max_ = std::max(max_, x);
  }

  long min() const noexcept { return min_; }
  long max() const noexcept { return max_; }
private:
  long min_ { std::numeric_limits<long>::max() };
  long max_ { std::numeric_limits<long>::min() };
};

class string_reducer {
public:
  string_reducer() noexcept {}
  void operator()(const std::string & x) noexcept {
    count_ += x.size();
  }

  long count() const noexcept { return count_; }
private:
  long count_ = 0;
};

#endif
