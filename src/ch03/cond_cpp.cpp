
#include "errors.hpp"

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

using ClockType    = std::chrono::high_resolution_clock;
using DurationType = std::chrono::duration<double, std::milli>;

struct my_struct_tag {
  std::mutex mutex;             /* Protects access to value */
  std::condition_variable cond; /* Signals change to value */
  int value = 0;                /* Access protected by mutex */
};

using my_struct_t = my_struct_tag;

my_struct_t data;

DurationType hibernation(1000.0); /* Default to 1 second */

/*
 * Thread start routine. It will set the main thread's predicate
 * and signal the condition variable.
 */
void
wait_thread()
{
  int status;

  std::this_thread::sleep_for(hibernation);
  std::lock_guard<std::mutex> lk(data.mutex);

  data.value = 1; /* Set predicate */
  data.cond.notify_one();
}

int
main(int argc, char* argv[])
{
  /*
   * If an argument is specified, interpret it as the number
   * of seconds for wait_thread to sleep before signaling the
   * condition variable.  You can play with this to see the
   * condition wait below time out or wake normally.
   */
  if (argc > 1) {
    int xHibernation = atoi(argv[1]);
    hibernation      = DurationType(1000.0 * xHibernation);
  }

  /*
   * Create wait_thread.
   */
  std::thread t(wait_thread);

  /*
   * Wait on the condition variable for 2 seconds, or until
   * signaled by the wait_thread. Normally, wait_thread
   * should signal. If you raise "hibernation" above 2
   * seconds, it will time out.
   */
  auto timeout = ClockType::now() + std::chrono::seconds(2);

  {
    std::unique_lock<std::mutex> lk(data.mutex);

    if (!data.cond.wait_until(lk, timeout, []() { return data.value != 0; })) {
      std::cout << "Condition wait timed out.\n";
    }

    if (data.value != 0)
      std::cout << "Condition was signaled.\n";
  }

  t.join();

  return 0;
}
