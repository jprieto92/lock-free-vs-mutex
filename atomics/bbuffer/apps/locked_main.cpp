#include "locked_buffer.h"
#include "generators.h"
#include "reducers.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <iostream>


template <typename G, typename B>
class producer {
public:
  producer(G & g, B & b) : gen_(g), buffer_(b) {}
  void operator()();

private:
  G & gen_;
  B & buffer_;
};

template <typename G, typename B>
void producer<G,B>::operator()() {
  while (!gen_.finished()) {
    auto x = gen_();
    bool finished = gen_.finished();
    buffer_.put(x,finished);
  }
}

template <typename R, typename B>
class consumer {
public:
  consumer(R & r, B & b) : red_(r), buffer_(b) {}
  void operator()();

private:
  R & red_;
  B & buffer_;
};

template <typename R, typename B>
void consumer<R,B>::operator()() {
  bool end = false;
  while (!end) {
    auto x = buffer_.get();
    end = x.first;
    if (!end) red_(x.second);
  }
}

template <typename G, typename R, typename B>
void run_task(G & generator, R & reducer, B & buffer) {

  std::chrono::time_point<std::chrono::system_clock> start, end; 
  start = std::chrono::system_clock::now();
  producer<G,B> prod{generator,buffer};
  consumer<R,B> cons{reducer,buffer};

  std::thread tp{prod};
  std::thread tc{cons};

  tp.join();
  tc.join();
  end = std::chrono::system_clock::now();
 
  int elapsed_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds> (end-start).count();
 
  std::cout << "Elapsed time: " << elapsed_nanoseconds << "ns\n";
}

void test_random(int bsize, long nitems) {
  using namespace std;

  // Setup buffer, generator and reducer
  locked_buffer<long> buf{bsize};
  random_numeric_generator generator{nitems};
  if (generator.finished()) return;
  numeric_reducer reducer;

  // Run generation -> Reduction
  run_task(generator,reducer,buf);

  // Get the result
  cout << "Min: " << reducer.min() << endl;
  cout << "Max: " << reducer.max() << endl;
}

void test_file(int bsize, const std::string & fname) {
  using namespace std;

  // Setup buffer, generator and reducer
  locked_buffer<long> buf{bsize};
  file_generator<long> generator{fname};
  if (generator.finished()) return;
  numeric_reducer reducer;

  // Run generation -> Reduction
  run_task(generator,reducer,buf);

  // Get the result
  cout << "Min: " << reducer.min() << endl;
  cout << "Max: " << reducer.max() << endl;
}

void test_file_count(int bsize, const std::string & fname) {
  using namespace std;

  // Setup buffer, generator and reducer
  locked_buffer<string> buf{bsize};
  file_generator<string> generator{fname};
  if (generator.finished()) return;
  string_reducer reducer;

  // Run generation -> Reduction
  run_task(generator,reducer,buf);

  // Get the result
  cout << "Count: " << reducer.count() << endl;
}

int main(int argc, char ** argv) {
  using namespace std;
  if (argc !=4) {
    cerr << "Wrong arguments" << endl;
    cerr << "Valid formats: " << endl;
    cerr << "  " << argv[0] << " random busize nitems" << endl;
    cerr << "  " << argv[0] << " file busize filename" << endl;
    cerr << "  " << argv[0] << " count busize filename" << endl;
    return -1;
  }
   
  const string mode = argv[1];
  const int bsize = stoi(argv[2]);
  if (mode=="random") {
    const long nitems = stol(argv[3]);
    test_random(bsize, nitems);
  }
  else if(mode=="file") {
    const string name = argv[3];
    test_file(bsize, name);
  }
  else if (mode=="count") {
    const string name = argv[3];
    test_file_count(bsize, name);
  }
  else {
    cerr << "Unknown mode: " << mode << endl;
    return -2;
  }

  return 0;
}

