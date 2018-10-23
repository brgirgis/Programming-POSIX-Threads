/*
 * alarm_mutex.c
 *
 * This is an enhancement to the alarm_thread.c program, which
 * created an "alarm thread" for each alarm command. This new
 * version uses a single alarm thread, which reads the next
 * entry in a list. The main thread places new requests onto the
 * list, in order of absolute expiration time. The list is
 * protected by a mutex, and the alarm thread sleeps for at
 * least 1 second, each iteration, to ensure that the main
 * thread can lock the mutex to add new work to the list.
 */

#include "errors.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

using ClockType    = std::chrono::high_resolution_clock;
using DurationType = std::chrono::duration<double, std::milli>;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/*
 * The "alarm" structure now contains the time_t (time since the
 * Epoch, in seconds) for each alarm, so that they can be
 * sorted. Storing the requested number of seconds would not be
 * enough, since the "alarm thread" cannot tell how long it has
 * been on the list.
 */
struct alarm_tag {
  unsigned seconds = -1;
  ClockType::time_point time;
  std::string message;
};

using alarm_t = alarm_tag;

std::mutex alarm_mutex;
std::list<alarm_t> alarm_list;

/*
 * The alarm thread's start routine.
 */
void
alarm_thread()
{
  /*
   * Loop forever, processing commands. The alarm thread will
   * be disintegrated when the process exits.
   */
  while (true) {
    DurationType sleep_time(0.0);

    {
      std::lock_guard<std::mutex> l(alarm_mutex);

      auto now = ClockType::now();

      for (auto it = alarm_list.begin(); it != alarm_list.end();) {
        if (it->time <= now) {
          std::cout << it->message << std::endl;
          it = alarm_list.erase(it);
        }
        else {
          DurationType remaining = it->time - ClockType::now();
          sleep_time             = MIN(remaining, sleep_time);
          ++it;
        }
      }

      /*
       * Unlock the mutex before waiting, so that the main
       * thread can lock it to insert a new alarm request. If
       * the sleep_time is 0, then call sched_yield, giving
       * the main thread a chance to run if it has been
       * readied by user input, without delaying the message
       * if there's no input.
       */
    }

    if (sleep_time.count() > 0)
      std::this_thread::sleep_for(sleep_time);
    else
      std::this_thread::yield();
  }
}

int
main(int argc, char* argv[])
{
  std::thread t(alarm_thread);
  t.detach();

  std::string line;
  while (true) {
    std::cout << "Alarm> ";

    std::getline(std::cin, line);
    if (!std::cin.good())
      exit(0);

    if (line.empty())
      continue;

    std::istringstream in(line);

    alarm_t alarm;

    in >> alarm.seconds;
    alarm.message = in.str();

    if (alarm.seconds < 1 || alarm.message.empty()) {
      std::cout << "Bad command\n";
      continue;
    }

    alarm.time = ClockType::now() + std::chrono::seconds(alarm.seconds);

    std::lock_guard<std::mutex> l(alarm_mutex);

    /*
     * Insert the new alarm into the list of alarms.
     */
    alarm_list.push_back(alarm);

#ifdef DEBUG
    std::cout << "[list: ";
    for (const auto& iAlarm : alarm_list) {
      DurationType remaining = iAlarm.time - ClockType::now();
      std::cout << "[\"" << iAlarm.message << "\": " << remaining.count()
                << "], ";
    }
    std::cout << "]\n";
#endif
  }
}
