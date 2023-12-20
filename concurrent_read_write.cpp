// To compile: g++ -lpthread -std=c++17 -O2 test.cpp
// to run: a.out 100000 20
// example output:
// $./a.out 100000 20
// run with loop_count = 100000thread_count = 20
// Segmentation fault (core dumped)
#include <iostream>
#include <mutex>
#include <stdlib.h>
#include <thread>
#include <unordered_map>
#include <vector>
#include <shared_mutex>

class TestObj {
public:
  TestObj(const std::string &val) : val1_(val), val2_(val), val3_(val) {}

  std::string val1_;
  std::string val2_;
  std::string val3_;
};

std::mutex map_mtx;
//std::shared_mutex map_mtx;
std::unordered_map<std::string, TestObj> global_map;
// key used by WriteToMap.
uint64_t unique_key = 0;

void WriteToMap(int loop_count) {
  while ((--loop_count) > 0) {
				// take lock so every write would only add a new entry into the map
    std::unique_lock write_lock(map_mtx);
    std::string test_obj_val = std::to_string(rand());
    TestObj new_obj = TestObj(test_obj_val);
    global_map.insert(std::make_pair(std::to_string(unique_key), new_obj));
    unique_key++;
  }
}

void ReadMap(int loop_count) {
  while ((--loop_count) > 0) {
		//		std::shared_lock read_lock(map_mtx);
    //std::unique_lock lock(map_mtx);
    auto it = global_map.find(std::to_string(rand() % loop_count));
    if (it != global_map.end()) {
      // referencing the member variables
      auto &testobj = it->second;
      std::string res = testobj.val1_ + testobj.val2_ + testobj.val3_;
      if (res.empty()) {
        std::cout << "empty result for " << it->first << "\n";
      }
    }
  }
}

int main(int argc, char *argv[]) {
  int loop_count = std::stoi(argv[1]);
  int thread_count = std::stoi(argv[2]);
  std::cout << "run with loop_count = " << std::to_string(loop_count)
            << "thread_count = " << std::to_string(thread_count) << "\n";
  std::vector<std::thread> thrds;
  for (int i = 0; i < thread_count; ++i) {
    thrds.push_back(std::thread{&WriteToMap, loop_count});
    thrds.push_back(std::thread{&ReadMap, loop_count});
  }

  for (auto &thrd : thrds) {
    thrd.join();
  }
  return 0;
}
