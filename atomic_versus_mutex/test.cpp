#include <chrono>
#include <iostream>
#include <mutex>
#include <atomic>
#include <thread>

typedef std::chrono::high_resolution_clock Clock;

std::mutex mtx;
bool tmp = false;
std::atomic<bool> atomic_val {tmp};

/* 
 * Test performance difference between atomic.compare_exchange_strong with locking mutex.
 * Compile with -O3 would eliminate the loop within RunMutexTest.
 * Compile with `g++ -O2 -pthread -std=c++17 test.cpp`
 *
 * NOTE:
 * Need to link to pthread library, otherwise, cpp would cheat by not doing the real locking,
 * making mutex appears to be faster, while actually, according to this test, atomic cas
 * is 1.3-1.4X faster.
 */

int RunMutexTest(uint64_t run_cnt) {
  for (uint64_t i = 0; i < run_cnt; ++i) {
    std::lock_guard<std::mutex> guard(mtx);
    tmp = !tmp;
  }
  return 0;
}

int RunAtomicTest(uint64_t run_cnt) {
  for (uint64_t i = 0; i < run_cnt; ++i) {
    // simulate lock and unlock;
    atomic_val.compare_exchange_strong(tmp, !tmp, std::memory_order_acq_rel, std::memory_order_acquire);
    tmp = !tmp;
    atomic_val.store(tmp, std::memory_order_release);
  }
  return 0;
}

int main() {
  uint64_t run_cnt = 1 << 26;
  std::cout << "Perform lock/CAS for " << run_cnt << " times each\n";

  auto start_time = Clock::now();
  RunMutexTest(run_cnt);
  auto end_time = Clock::now();
  auto micro_secs = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
  std::cout << "using std::mutex took : " << 1e-6*micro_secs << " secs\n";

  start_time = Clock::now();
  RunAtomicTest(run_cnt);
  end_time = Clock::now();
  micro_secs = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
  std::cout << "using atomic.compare_exchange_strong took : " << 1e-6*micro_secs << " secs\n";
  return 0;
}
