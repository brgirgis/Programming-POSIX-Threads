
//#include "errors.hpp"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

using ClockType    = std::chrono::high_resolution_clock;
using DurationType = std::chrono::duration<double>;
using TimePoint    = ClockType::time_point;

/*
 * The "alarm" structure now contains the time_t (time since the
 * Epoch, in seconds) for each alarm, so that they can be
 * sorted. Storing the requested number of seconds would not be
 * enough, since the "alarm thread" cannot tell how long it has
 * been on the list.
 */
struct alarm_tag {
  unsigned seconds = -1;
  TimePoint time;
  std::string message;
};

using alarm_t = alarm_tag;
using alarm_p = std::unique_ptr<alarm_t>;

std::mutex alarm_mutex;
std::condition_variable alarm_cond;
std::list<alarm_p> alarm_list;

const TimePoint FarFuture =
    ClockType::now() + std::chrono::hours(365 * 24 * 10);
TimePoint current_alarm = FarFuture;

/*
 * Insert alarm entry on list, in order.
 */
void
alarm_insert(alarm_p alarm)
{
  /*
   * LOCKING PROTOCOL:
   *
   * This routine requires that the caller have locked the
   * alarm_mutex!
   */

  auto alarm_ptr = alarm.get();

  auto it = alarm_list.insert(std::find_if(alarm_list.begin(),
                                           alarm_list.end(),
                                           [&alarm_ptr](const auto& a) {
                                             return a->time > alarm_ptr->time;
                                           }),
                              std::move(alarm));

#if 1  // DEBUG
  std::cout << "[list: ";
  for (const auto& iAlarm : alarm_list) {
    DurationType remaining = iAlarm->time - ClockType::now();
    std::cout << "[\"" << iAlarm->message << "\": " << remaining.count()
              << "], ";
  }
  std::cout << "]\n";
#endif

  /*
   * Wake the alarm thread if it is not busy (that is, if
   * current_alarm is 0, signifying that it's waiting for
   * work), or if the new alarm comes before the one on
   * which the alarm thread is waiting.
   */
  if ((*it)->time < current_alarm) {
    current_alarm = (*it)->time;
    alarm_cond.notify_one();
  }
}

/*
 * The alarm thread's start routine.
 */
void
alarm_thread()
{
  /*
   * Loop forever, processing commands. The alarm thread will
   * be disintegrated when the process exits. Lock the mutex
   * at the start -- it will be unlocked during condition
   * waits, so the main thread can insert alarms.
   */
  std::unique_lock<std::mutex> lk(alarm_mutex);

  while (true) {
    /*
     * If the alarm list is empty, wait until an alarm is
     * added. Setting current_alarm to 0 informs the insert
     * routine that the thread is not busy.
     */
    current_alarm = FarFuture;
    alarm_cond.wait(lk, []() { return !alarm_list.empty(); });

    auto now = ClockType::now();

    bool expired = false;

    auto it           = alarm_list.begin();
    const auto& alarm = *it;

    if (alarm->time > now) {
#if 1  // def DEBUG
      std::cout << "waiting for \"" << alarm->message
                << "\":" << DurationType(alarm->time - now).count()
                << std::endl;
#endif

      current_alarm = alarm->time;
      if (alarm_cond.wait_until(lk, alarm->time) == std::cv_status::timeout) {
        expired = true;
      }
    }
    else
      expired = true;
    if (expired) {
      std::cout << alarm->message << std::endl;
      alarm_list.erase(it);
    }
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

    alarm_p alarm(new alarm_t());

    in >> alarm->seconds;
    alarm->message = in.str();

    if (alarm->seconds < 1 || alarm->message.empty()) {
      std::cout << "Bad command\n";
      continue;
    }

    alarm->time = ClockType::now() + std::chrono::seconds(alarm->seconds);

    std::lock_guard<std::mutex> l(alarm_mutex);

    alarm_insert(std::move(alarm));
  }
}
