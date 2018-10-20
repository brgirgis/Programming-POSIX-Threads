
#include "errors.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

#define ITERATIONS 10

/*
 * Initialize a static array of 3 mutexes.
 */
std::mutex mutex[3];

int backoff    = 1; /* Whether to backoff or deadlock */
int yield_flag = 0; /* 0: no yield, >0: yield, <0: sleep */

/*
 * This is a thread start routine that locks all mutexes in
 * order, to ensure a conflict with lock_reverse, which does the
 * opposite.
 */

void
lock_forward_new()
{
  for (int iterate = 0; iterate < ITERATIONS; iterate++) {
    std::lock(mutex[0], mutex[1], mutex[2]);
    std::lock_guard<std::mutex> l0(mutex[0], std::adopt_lock);
    std::lock_guard<std::mutex> l1(mutex[1], std::adopt_lock);
    std::lock_guard<std::mutex> l2(mutex[2], std::adopt_lock);
    std::cout << "new lock forward got all locks" << std::endl;
  }
}

void
lock_forward()
{
  for (int iterate = 0; iterate < ITERATIONS; iterate++) {
    int backoffs = 0;
    for (int i = 0; i < 3; i++) {
      if (i == 0) {
        mutex[i].lock();
      }
      else {
        bool isFree = true;
        if (backoff)
          isFree = mutex[i].try_lock();
        else
          mutex[i].lock();
        if (!isFree) {
          backoffs++;
          std::cout << " [forward locker backing off at " << i << "]"
                    << std::endl;
          for (; i >= 0; i--) {
            mutex[i].unlock();
          }
        }
        else {
          std::cout << " forward locker got " << i << std::endl;
        }
      }
      /*
       * Yield processor, if needed to be sure locks get
       * interleaved on a uniprocessor.
       */
      if (yield_flag) {
        if (yield_flag > 0)
          std::this_thread::yield();
        else
          std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
    /*
     * Report that we got 'em, and unlock to try again.
     */
    std::cout << "lock forward got all locks, " << backoffs << " backoffs"
              << std::endl;
    mutex[2].unlock();
    mutex[1].unlock();
    mutex[0].unlock();
    std::this_thread::yield();
  }
}

void
lock_backward_new()
{
  for (int iterate = 0; iterate < ITERATIONS; iterate++) {
    std::lock(mutex[2], mutex[1], mutex[0]);
    std::lock_guard<std::mutex> l2(mutex[2], std::adopt_lock);
    std::lock_guard<std::mutex> l1(mutex[1], std::adopt_lock);
    std::lock_guard<std::mutex> l0(mutex[0], std::adopt_lock);
    std::cout << "new lock backward got all locks" << std::endl;
  }
}

/*
 * This is a thread start routine that locks all mutexes in
 * reverse order, to ensure a conflict with lock_forward, which
 * does the opposite.
 */
void
lock_backward()
{
  for (int iterate = 0; iterate < ITERATIONS; iterate++) {
    int backoffs = 0;
    for (int i = 2; i >= 0; i--) {
      if (i == 2) {
        mutex[i].lock();
      }
      else {
        bool isFree = true;
        if (backoff)
          isFree = mutex[i].try_lock();
        else
          mutex[i].lock();
        if (!isFree) {
          backoffs++;
          std::cout << " [backward locker backing off at " << i << "]"
                    << std::endl;
          for (; i < 3; i++) {
            mutex[i].unlock();
          }
        }
        else {
          std::cout << " backward locker got " << i << std::endl;
        }
      }
      /*
       * Yield processor, if needed to be sure locks get
       * interleaved on a uniprocessor.
       */
      if (yield_flag) {
        if (yield_flag > 0)
          std::this_thread::yield();
        else
          std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
    /*
     * Report that we got 'em, and unlock to try again.
     */
    std::cout << "lock backward got all locks, " << backoffs << " backoffs"
              << std::endl;
    mutex[0].unlock();
    mutex[1].unlock();
    mutex[2].unlock();
    std::this_thread::yield();
  }
}

int
main(int argc, char* argv[])
{
  /*
   * If the first argument is absent, or nonzero, a backoff
   * algorithm will be used to avoid deadlock. If the first
   * argument is zero, the program will deadlock on a lock
   * "collision."
   */
  if (argc > 1)
    backoff = atoi(argv[1]);

  /*
   * If the second argument is absent, or zero, the two
   * threads run "at speed." On some systems, especially
   * uniprocessors, one thread may complete before the other
   * has a chance to run, and you won't see a deadlock or
   * backoffs. In that case, try running with the argument set
   * to a positive number to cause the threads to call
   * sched_yield() at each lock; or, to make it even more
   * obvious, set to a negative number to cause the threads to
   * call sleep(1) instead.
   */
  if (argc > 2)
    yield_flag = atoi(argv[2]);

  if (backoff < 0) {
    std::thread t1(lock_forward_new);
    std::thread t2(lock_backward_new);

    t1.join();
    t2.join();
    return 0;
  }

  std::thread t1(lock_forward);
  std::thread t2(lock_backward);

  t1.join();
  t2.join();
}
