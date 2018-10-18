# Digital UNIX 4.0 compilation flags:
#CFLAGS=-std1 -pthread -g -w1 $(DEBUGFLAGS)
#RTFLAGS=-lrt

# Solaris 2.5 compilation flags:
#CFLAGS=-D_POSIX_C_SOURCE=199506 -D_REENTRANT -Xa -lpthread -g $(DEBUGFLAGS)
#RTFLAGS=-lposix4

# Linux compilation flags:
CFLAGS=-pthread -O3 $(DEBUGFLAGS)
RTFLAGS=-lrt

BIN=./bin
SOURCE=./src

INC=-I$(SOURCE)

SOURCES_C=\
ch01/alarm.c \
ch01/alarm_fork.c \
ch01/alarm_thread.c \
ch01/thread_error.c \
ch02/hello.c \
ch02/thread.c \
ch02/lifecycle.c \
ch03/alarm_cond.c \
ch03/alarm_mutex.c \
ch03/backoff.c \
ch03/cond_dynamic.c \
ch03/cond_static.c \
ch03/cond.c \
ch03/mutex_dynamic.c \
ch03/mutex_static.c \
ch03/trylock.c \
ch04/crew.c \
ch04/pipe.c \
ch04/server.c \
ch05/cancel_async.c \
ch05/cancel_cleanup.c \
ch05/cancel_disable.c \
ch05/cancel_subcontract.c \
ch05/cancel.c \
ch05/cond_attr.c \
ch05/mutex_attr.c \
ch05/once.c \
ch05/sched_attr.c \
ch05/sched_thread.c \
ch05/thread_attr.c \
ch05/tsd_destructor.c \
ch05/tsd_once.c \
ch06/atfork.c \
ch06/flock.c \
ch06/getlogin.c \
ch06/putchar.c \
ch06/semaphore_signal.c \
ch06/semaphore_wait.c \
ch06/sigev_thread.c \
ch06/sigwait.c \
ch06/susp.c \
ch07/barrier_main.c \
ch07/rwlock_main.c \
ch07/rwlock_try_main.c \
ch07/workq_main.c \
ch08/inertia.c

NAMES_C=$(SOURCES_C:.c=)
PROGRAMS_C=$(addprefix $(BIN)/, $(NAMES_C))

SOURCES_CXX=\
ch01/alarm_cpp.cpp \
ch01/alarm_fork_cpp.cpp \
ch01/alarm_thread_cpp.cpp \
ch03/alarm_mutex_cpp.cpp \
ch04/pipe_cpp.cpp

NAMES_CXX=$(SOURCES_CXX:.cpp=)
PROGRAMS_CXX=$(addprefix $(BIN)/, $(NAMES_CXX))

all:	format	dirs	${PROGRAMS_C}	${PROGRAMS_CXX}

format:
	#clang-format -i $(SOURCE)/*.c*
	clang-format -i $(SOURCE)/*.h*
	clang-format -i $(SOURCE)/*/*.c*
	clang-format -i $(SOURCE)/*/*.h*

dirs:
	mkdir -p $(BIN)
	mkdir -p $(BIN)/ch01 $(BIN)/ch02 $(BIN)/ch03 $(BIN)/ch04 $(BIN)/ch05
	mkdir -p $(BIN)/ch06 $(BIN)/ch07 $(BIN)/ch08

$(BIN)/%:	$(SOURCE)/%.c
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

$(BIN)/ch03/alarm_mutex:	$(SOURCE)/ch03/alarm_mutex.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS)

$(BIN)/ch03/backoff:	$(SOURCE)/ch03/backoff.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS)
 
$(BIN)/ch05/sched_attr:	$(SOURCE)/ch05/sched_attr.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS)
 
$(BIN)/ch05/sched_thread:	$(SOURCE)/ch05/sched_thread.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS) 

$(BIN)/ch06/semaphore_signal:	$(SOURCE)/ch06/semaphore_signal.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS) 

$(BIN)/ch06/semaphore_wait:	$(SOURCE)/ch06/semaphore_wait.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS) 

$(BIN)/ch06/sigev_thread:	$(SOURCE)/ch06/sigev_thread.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS) 

$(BIN)/ch06/susp:	$(SOURCE)/ch06/susp.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS) 

$(BIN)/ch07/rwlock_main: $(SOURCE)/ch07/rwlock.c $(SOURCE)/ch07/rwlock.h $(SOURCE)/ch07/rwlock_main.c
	${CC} $(INC) ${CFLAGS} ${LDFLAGS} -o $@ $(SOURCE)/ch07/rwlock_main.c $(SOURCE)/ch07/rwlock.c

$(BIN)/ch07/rwlock_try_main: $(SOURCE)/ch07/rwlock.h $(SOURCE)/ch07/rwlock.c $(SOURCE)/ch07/rwlock_try_main.c
	${CC} $(INC) ${CFLAGS} ${LDFLAGS} -o $@ $(SOURCE)/ch07/rwlock_try_main.c $(SOURCE)/ch07/rwlock.c

$(BIN)/ch07/barrier_main: $(SOURCE)/ch07/barrier.h $(SOURCE)/ch07/barrier.c $(SOURCE)/ch07/barrier_main.c
	${CC} $(INC) ${CFLAGS} ${LDFLAGS} -o $@ $(SOURCE)/ch07/barrier_main.c $(SOURCE)/ch07/barrier.c

$(BIN)/ch07/workq_main: $(SOURCE)/ch07/workq.h $(SOURCE)/ch07/workq.c $(SOURCE)/ch07/workq_main.c
	${CC} $(INC) ${CFLAGS} ${RTFLAGS} ${LDFLAGS} -o $@ $(SOURCE)/ch07/workq_main.c $(SOURCE)/ch07/workq.c

$(BIN)/%:	$(SOURCE)/%.cpp
	$(CXX) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

clean:
	@rm -rf $(PROGRAMS_C) ${PROGRAMS_CXX} *.o

recompile:	clean all
