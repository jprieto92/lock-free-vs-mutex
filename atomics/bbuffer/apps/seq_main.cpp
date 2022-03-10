#include "seq_buffer.h"
#include "generators.h"
#include "reducers.h"
#include <utility>

#include <iostream>
#include <chrono>

// Run the sequential task for a Generator, a Reducer and a buffer
// This task puts elements in the buffer until it is full and then
// gets them from the buffer.
// When the producer finishes generation it puts a finished indication.
// When the consumer finds the indication it also finishes. 
template <typename G, typename R, typename B>
void run_task(G & generator, R & reducer, B & buffer) {
  std::chrono::time_point<std::chrono::system_clock> start, end; 
  start = std::chrono::system_clock::now();
  for (;;) {
    while (!buffer.full()) {
      auto x = generator();
      bool finished = generator.finished();
      buffer.put(x, finished);
      if (finished) break;
    }
    while (!buffer.empty()) {
      auto x = buffer.get();
      if (x.first) goto end_task;
      reducer(x.second);
    }
  }
  end_task: {}
  end = std::chrono::system_clock::now();
 
  int elapsed_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds> (end-start).count();
 
  std::cout << "Elapsed time: " << elapsed_nanoseconds << "ns\n";
}

void test_random(int bsize, long nitems) {
  using namespace std;

  // Setup buffer, generator and reducer
  seq_buffer<long> buf{bsize};
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
  seq_buffer<long> buf{bsize};
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
  seq_buffer<string> buf{bsize};
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
