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

SOURCES_C=alarm.c	alarm_cond.c	alarm_fork.c	alarm_mutex.c	\
	alarm_thread.c	atfork.c	backoff.c	\
	barrier_main.c	cancel.c	cancel_async.c	cancel_cleanup\
	cancel_disable.c cancel_subcontract.c	cond.c	cond_attr.c	\
	crew.c cond_dynamic.c	cond_static.c	flock.c	getlogin.c hello.c \
	inertia.c	lifecycle.c	mutex_attr.c	\
	mutex_dynamic.c	mutex_static.c	once.c	pipe.c	putchar.c	\
	rwlock_main.c	rwlock_try_main.c		\
	sched_attr.c	sched_thread.c	semaphore_signal.c	\
	semaphore_wait.c	server.c	sigev_thread.c	\
	sigwait.c	susp.c	thread.c \
	thread_attr.c	thread_error.c	trylock.c	tsd_destructor.c \
	tsd_once.c	workq_main.c

NAMES_C=$(SOURCES_C:.c=)
PROGRAMS_C=$(addprefix $(BIN)/c_, $(NAMES_C))

SOURCES_CXX=alarm_mutex.cpp	pipe.cpp	

NAMES_CXX=$(SOURCES_CXX:.cpp=)
PROGRAMS_CXX=$(addprefix $(BIN)/cpp_, $(NAMES_CXX))

all:	${PROGRAMS_C}	${PROGRAMS_CXX}

$(BIN)/c_%:	$(SOURCE)/%.c
	$(CC) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

$(BIN)/c_alarm_mutex:	$(SOURCE)/alarm_mutex.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS)

$(BIN)/c_backoff:	$(SOURCE)/backoff.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS)
 
$(BIN)/c_sched_attr:	$(SOURCE)/sched_attr.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS)
 
$(BIN)/c_sched_thread:	$(SOURCE)/sched_thread.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS) 

$(BIN)/c_semaphore_signal:	$(SOURCE)/semaphore_signal.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS) 

$(BIN)/c_semaphore_wait:	$(SOURCE)/semaphore_wait.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS) 

$(BIN)/c_sigev_thread:	$(SOURCE)/sigev_thread.c
	$(CC) $(INC) $< $(CFLAGS) ${RTFLAGS} -o $@ $(LIBS) 

$(BIN)/c_susp:
	${CC} ${CFLAGS} ${RTFLAGS} ${LDFLAGS} -o $@ $(SOURCE)/susp.c

$(BIN)/c_rwlock_main: $(SOURCE)/rwlock.c $(SOURCE)/rwlock.h $(SOURCE)/rwlock_main.c
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $(SOURCE)/rwlock_main.c $(SOURCE)/rwlock.c

$(BIN)/c_rwlock_try_main: $(SOURCE)/rwlock.h $(SOURCE)/rwlock.c $(SOURCE)/rwlock_try_main.c
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $(SOURCE)/rwlock_try_main.c $(SOURCE)/rwlock.c

$(BIN)/c_barrier_main: $(SOURCE)/barrier.h $(SOURCE)/barrier.c $(SOURCE)/barrier_main.c
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $(SOURCE)/barrier_main.c $(SOURCE)/barrier.c

$(BIN)/c_workq_main: $(SOURCE)/workq.h $(SOURCE)/workq.c $(SOURCE)/workq_main.c
	${CC} ${CFLAGS} ${RTFLAGS} ${LDFLAGS} -o $@ $(SOURCE)/workq_main.c $(SOURCE)/workq.c

$(BIN)/cpp_%:	$(SOURCE)/%.cpp
	$(CXX) $(INC) $< $(CFLAGS) -o $@ $(LIBS)

clean:
	@rm -rf $(PROGRAMS_C) ${PROGRAMS_CXX} *.o

recompile:	clean all
