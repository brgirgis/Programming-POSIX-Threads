
#include "errors.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

#define SPIN 10000000

using ClockType = std::chrono::high_resolution_clock;

std::mutex mutex;
long counter;
ClockType::time_point end_time;

/*
 * Thread start routine that repeatedly locks a mutex and
 * increments a counter.
 */
void
counter_thread()
{
  /*
   * Until end_time, increment the counter each
   * second. Instead of just incrementing the counter, it
   * sleeps for another second with the mutex locked, to give
   * monitor_thread a reasonable chance of running.
   */
  while (ClockType::now() < end_time) {
    {
      std::lock_guard<std::mutex> l(mutex);
      for (long spin = 0; spin < SPIN; spin++) counter++;
      // std::this_thread::sleep_for(std::chrono::seconds(4));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  std::cout << "[Counter] total spins " << counter << std::endl;
}

/*
 * Thread start routine to "monitor" the counter. Every 3
 * seconds, try to lock the mutex and read the counter. If the
 * trylock fails, skip this cycle.
 */
void
monitor_thread()
{
  /*
   * Loop until end_time, checking the counter every 3
   * seconds.
   */
  int misses = 0;
  while (ClockType::now() < end_time) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    if (mutex.try_lock()) {
      std::lock_guard<std::mutex> l(mutex, std::adopt_lock);
      std::cout << "[Monitor] Counter is " << counter / SPIN << std::endl;
    }
    else
      misses++; /* Count "misses" on the lock */
  }

  std::cout << "[Monitor] thread missed update " << misses << " times.\n";
}

int
main(int argc, char* argv[])
{
  end_time = ClockType::now() + std::chrono::seconds(60); /* Run for 1 minute */
  std::thread t1(counter_thread);
  std::thread t2(monitor_thread);

  t1.join();
  t2.join();

  return 0;
}
