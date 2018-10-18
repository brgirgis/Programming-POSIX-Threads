/*
 * lifecycle.c
 *
 * Demonstrate the "life cycle" of a typical thread. A thread is
 * created, and then joined.
 */
#include <pthread.h>
#include "errors.h"

/*
 * Thread start routine.
 */
void*
thread_routine(void* arg)
{
  printf("Inside thread %i\n", pthread_self());
  return arg;
}

main(int argc, char* argv[])
{
  pthread_t thread_id;
  void* thread_result;
  int status;

  printf("Inside thread %i\n", pthread_self());
  status = pthread_create(&thread_id, NULL, thread_routine, NULL);
  if (status != 0)
    err_abort(status, "Create thread");
  sleep(1.0);
  printf("Main thread created thread %i\n", thread_id);

  status = pthread_join(thread_id, &thread_result);
  if (status != 0)
    err_abort(status, "Join thread");
  if (thread_result == NULL)
    return 0;
  else
    return 1;
}
