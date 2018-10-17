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
#include <pthread.h>
#include <time.h>
#include "errors.h"

#include <list>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/*
 * The "alarm" structure now contains the time_t (time since the
 * Epoch, in seconds) for each alarm, so that they can be
 * sorted. Storing the requested number of seconds would not be
 * enough, since the "alarm thread" cannot tell how long it has
 * been on the list.
 */
struct alarm_tag {
  int seconds;
  time_t time; /* seconds from EPOCH */
  char message[64];
};

using alarm_t = alarm_tag;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
std::list<alarm_t> alarm_list;

/*
 * The alarm thread's start routine.
 */
void *alarm_thread(void *arg) {
  int sleep_time;
  time_t now;
  int status;

  /*
   * Loop forever, processing commands. The alarm thread will
   * be disintegrated when the process exits.
   */
  while (1) {
    status = pthread_mutex_lock(&alarm_mutex);
    if (status != 0) err_abort(status, "Lock mutex");

    now = time(NULL);
    sleep_time = 0;

    for (auto it = alarm_list.begin(); it != alarm_list.end();) {
      if (it->time <= now) {
        printf("(%d) %s\n", it->seconds, it->message);
        it = alarm_list.erase(it);
      } else {
        sleep_time = MIN(it->time - now, sleep_time);
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
    status = pthread_mutex_unlock(&alarm_mutex);
    if (status != 0) err_abort(status, "Unlock mutex");
    if (sleep_time > 0)
      sleep(sleep_time);
    else
      sched_yield();
  }
}

int main(int argc, char *argv[]) {
  int status;
  char line[128];
  pthread_t thread;

  status = pthread_create(&thread, NULL, alarm_thread, NULL);
  if (status != 0) err_abort(status, "Create alarm thread");
  while (1) {
    printf("alarm> ");
    if (fgets(line, sizeof(line), stdin) == NULL) exit(0);
    if (strlen(line) <= 1) continue;
    alarm_t alarm;

    /*
     * Parse input line into seconds (%d) and a message
     * (%64[^\n]), consisting of up to 64 characters
     * separated from the seconds by whitespace.
     */
    if (sscanf(line, "%d %64[^\n]", &alarm.seconds, alarm.message) < 2) {
      fprintf(stderr, "Bad command\n");
    } else {
      alarm.time = time(NULL) + alarm.seconds;

      status = pthread_mutex_lock(&alarm_mutex);
      if (status != 0) err_abort(status, "Lock mutex");

      /*
       * Insert the new alarm into the list of alarms.
       */
      alarm_list.push_back(alarm);

#ifdef DEBUG
      printf("[list: ");
      for (const auto &iAlarm : alarm_list)
        printf("%d(%d)[\"%s\"] ", iAlarm.time, iAlarm.time - time(NULL),
               iAlarm.message);
      printf("]\n");
#endif
      status = pthread_mutex_unlock(&alarm_mutex);
      if (status != 0) err_abort(status, "Unlock mutex");
    }
  }
}
