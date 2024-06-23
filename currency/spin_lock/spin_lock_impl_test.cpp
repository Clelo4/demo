#include <chrono>
#include <emmintrin.h>
#include <iostream>
#include <numeric> // 用于计算平均值
#include <thread>
#include <vector>

std::atomic<bool> flag(false);
std::atomic<long> count(0);

// 空循环的自旋等待
void spin_wait_empty() {
  while (!flag.load(std::memory_order_relaxed)) {
    // 空循环体
  }
}

// 使用_mm_pause()的自旋等待
void spin_wait_mm_pause() {
  while (!flag.load(std::memory_order_relaxed)) {
    _mm_pause();
  }
}

int main() {
  const int num_trials = 100; // 进行10轮测试
  std::vector<double> empty_times(num_trials);
  std::vector<double> mm_pause_times(num_trials);

  for (int i = 0; i < num_trials; ++i) {
    // 测试空循环的性能
    auto start_empty = std::chrono::high_resolution_clock::now();
    std::thread t1(spin_wait_empty);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(10)); // 让自旋等待一段时间
    flag.store(true, std::memory_order_relaxed);
    t1.join();
    auto end_empty = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_empty =
        end_empty - start_empty;
    empty_times[i] = elapsed_empty.count();

    // 测试使用_mm_pause()的性能
    auto start_mm_pause = std::chrono::high_resolution_clock::now();
    std::thread t2(spin_wait_mm_pause);
    std::this_thread::sleep_for(
        std::chrono::milliseconds(10)); // 让自旋等待一段时间
    flag.store(true, std::memory_order_relaxed);
    t2.join();
    auto end_mm_pause = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_mm_pause =
        end_mm_pause - start_mm_pause;
    mm_pause_times[i] = elapsed_mm_pause.count();
  }

  // 计算平均执行时间
  double avg_empty_time =
      std::accumulate(empty_times.begin(), empty_times.end(), 0.0) / num_trials;
  double avg_mm_pause_time =
      std::accumulate(mm_pause_times.begin(), mm_pause_times.end(), 0.0) /
      num_trials;

  // 输出平均执行时间
  std::cout << "Average time for empty loop spin wait: " << avg_empty_time
            << " ms\n";
  std::cout << "Average time for _mm_pause() spin wait: " << avg_mm_pause_time
            << " ms\n";

  return 0;
}
