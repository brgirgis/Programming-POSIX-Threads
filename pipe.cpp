/*
 * pipe.c
 *
 * Simple demonstration of a pipeline. main() is a loop that
 * feeds the pipeline with integer values. Each stage of the
 * pipeline increases the integer by one before passing it along
 * to the next. Entering the command "=" reads the pipeline
 * result. (Notice that too many '=' commands will hang.)
 */
#include <pthread.h>
#include "errors.h"

#include <iostream>
#include <list>
#include <thread>

/*
 * Internal structure describing a "stage" in the
 * pipeline. One for each thread, plus a "result
 * stage" where the final thread can stash the value.
 */
struct stage_tag {
  pthread_mutex_t mutex;       /* Protect data */
  pthread_cond_t dataIsAvail;  /* Data available */
  pthread_cond_t threadIsIdle; /* Ready for data */
  bool isDataAvail;            /* Data present */
  long data;                   /* Data to process */
  pthread_t thread;            /* Thread for stage */
  stage_tag *next;
};

using stage_t = stage_tag;

using StageList = std::list<stage_t>;

/*
 * External structure representing the entire
 * pipeline.
 */
struct pipe_tag {
  pthread_mutex_t mutex; /* Mutex to protect pipe */
  StageList stageList;   /* stages list */
  int nActive;           /* Active data elements */
};

using pipe_t = pipe_tag;

/*
 * Internal function to send a "message" to the
 * specified pipe stage. Threads use this to pass
 * along the modified data item.
 */
int pipe_send(stage_t &stage, long data) {
  int status;

  status = pthread_mutex_lock(&stage.mutex);
  if (status != 0) return status;

  /*
   * If there's data in the pipe stage, wait for it
   * to be consumed.
   */
  while (stage.isDataAvail) {
    status = pthread_cond_wait(&stage.threadIsIdle, &stage.mutex);
    if (status != 0) {
      pthread_mutex_unlock(&stage.mutex);
      return status;
    }
  }

  /*
   * Send the new data
   */
  stage.data = data;
  stage.isDataAvail = true;

  status = pthread_cond_signal(&stage.dataIsAvail);
  if (status != 0) {
    pthread_mutex_unlock(&stage.mutex);
    return status;
  }

  status = pthread_mutex_unlock(&stage.mutex);
  return status;
}

/*
 * The thread start routine for pipe stage threads.
 * Each will wait for a data item passed from the
 * caller or the previous stage, modify the data
 * and pass it along to the next (or final) stage.
 */
void *pipe_stage(void *arg) {
  stage_t &stage = *(stage_t *)arg;
  stage_t &nextStage = *stage.next;
  int status;

  status = pthread_mutex_lock(&stage.mutex);
  if (status != 0) err_abort(status, "Lock pipe stage");

  while (1) {
    while (!stage.isDataAvail) {
      status = pthread_cond_wait(&stage.dataIsAvail, &stage.mutex);
      if (status != 0) err_abort(status, "Wait for previous stage");
    }

    pipe_send(nextStage, stage.data + 1);

    stage.isDataAvail = false;

    status = pthread_cond_signal(&stage.threadIsIdle);
    if (status != 0) err_abort(status, "Wake next stage");
  }

  /*
   * Notice that the routine never unlocks the stage->mutex.
   * The call to pthread_cond_wait implicitly unlocks the
   * mutex while the thread is waiting, allowing other threads
   * to make progress. Because the loop never terminates, this
   * function has no need to unlock the mutex explicitly.
   */
}

/*
 * External interface to create a pipeline. All the
 * data is initialized and the threads created. They'll
 * wait for data.
 */
int pipe_create(pipe_t &pipe, size_t nStages) {
  int status = pthread_mutex_init(&pipe.mutex, NULL);
  if (status != 0) err_abort(status, "Init pipe mutex");

  pipe.stageList.resize(nStages);
  pipe.nActive = 0;

  for (auto &iStage : pipe.stageList) {
    status = pthread_mutex_init(&iStage.mutex, NULL);
    if (status != 0) err_abort(status, "Init stage mutex");
    status = pthread_cond_init(&iStage.dataIsAvail, NULL);
    if (status != 0) err_abort(status, "Init dataIsAvail condition");
    status = pthread_cond_init(&iStage.threadIsIdle, NULL);
    if (status != 0) err_abort(status, "Init threadIsIdle condition");
    iStage.isDataAvail = false;
  }

  /*
   * Create the threads for the pipe stages only after all
   * the data is initialized (including all links). Note
   * that the last stage doesn't get a thread, it's just
   * a receptacle for the final pipeline value.
   *
   * At this point, proper cleanup on an error would take up
   * more space than worthwhile in a "simple example", so
   * instead of cancelling and detaching all the threads
   * already created, plus the synchronization object and
   * memory cleanup done for earlier errors, it will simply
   * abort.
   */
  for (auto it = pipe.stageList.begin(), ite = --pipe.stageList.end();
       it != ite;) {
    auto iit = it++;
    iit->next = &(*it);

    status = pthread_create(&iit->thread, NULL, pipe_stage, (void *)&*iit);
    if (status != 0) err_abort(status, "Create pipe stage");
  }
  return 0;
}

/*
 * Collect the result of the pipeline. Wait for a
 * result if the pipeline hasn't produced one.
 */
int pipe_result(pipe_t &pipe) {
  int status;

  status = pthread_mutex_lock(&pipe.mutex);
  if (status != 0) err_abort(status, "Lock pipe mutex");

  bool isEmpty = false;
  if (pipe.nActive <= 0)
    isEmpty = true;
  else
    pipe.nActive--;

  status = pthread_mutex_unlock(&pipe.mutex);
  if (status != 0) err_abort(status, "Unlock pipe mutex");

  if (isEmpty) {
    printf("Pipe is empty\n");
    return 0;
  }

  auto &tail = pipe.stageList.back();

  status = pthread_mutex_lock(&tail.mutex);
  if (status != 0) err_abort(status, "Lock pipe tail mutex");

  while (!tail.isDataAvail) {
    pthread_cond_wait(&tail.dataIsAvail, &tail.mutex);
  }

  long result = tail.data;
  tail.isDataAvail = false;

  status = pthread_cond_signal(&tail.threadIsIdle);
  if (status != 0) err_abort(status, "Signal pipe tail threadIsIdle");

  status = pthread_mutex_unlock(&tail.mutex);
  if (status != 0) err_abort(status, "Unlock pipe tail mutex");

  printf("Result is %ld\n", result);

  return 1;
}

/*
 * External interface to start a pipeline by passing
 * data to the first stage. The routine returns while
 * the pipeline processes in parallel. Call the
 * pipe_result return to collect the final stage values
 * (note that the pipe will stall when each stage fills,
 * until the result is collected).
 */
void pipe_start(pipe_t &pipe, long value) {
  int status = pthread_mutex_lock(&pipe.mutex);
  if (status != 0) err_abort(status, "Lock pipe mutex");

  if (pipe.nActive > pipe.stageList.size() - 1) {
    status = pthread_mutex_unlock(&pipe.mutex);
    pipe_result(pipe);
    status = pthread_mutex_lock(&pipe.mutex);
  }

  pipe.nActive++;

  status = pthread_mutex_unlock(&pipe.mutex);
  if (status != 0) err_abort(status, "Unlock pipe mutex");

  pipe_send(pipe.stageList.front(), value);
}

/*
 * The main program to "drive" the pipeline...
 */
int main(int argc, char *argv[]) {
  pipe_t my_pipe;
  pipe_create(my_pipe, 2);

  printf("Enter integer values, or \"=\" for next result\n");

  char line[128];
  while (1) {
    printf("Data> ");
    if (fgets(line, sizeof(line), stdin) == NULL) exit(0);
    printf(line);
    if (strlen(line) <= 1) continue;
    if (strlen(line) <= 2 && line[0] == '=') {
      pipe_result(my_pipe);
    } else {
      long value;
      if (sscanf(line, "%ld", &value) < 1)
        fprintf(stderr, "Enter an integer value\n");
      else
        pipe_start(my_pipe, value);
    }
  }
}
