/***************************************************************************
 *
 * Sequential version of Matrix Inverse
 * An adapted version of the code by Hkan Grahn
 ***************************************************************************/

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// if (PRINT == 1), prints matrixes.
// if (PRINT == 2), prints matrixes + debug info.
int PRINT; /* print switch               */

// TODO: clean up comments and code etc. + remove debug printf's.

// ==============================================
//                 thread pool
// ==============================================

// define jobFunc type
typedef void (*jobFunc)(void *params);

// ==============
// pool job item
// ==============
struct pool_job {
  jobFunc func;
  void *params;
  struct pool_job *next; // linked list, next item
};
typedef struct pool_job pool_job_t;

// ===========
// thread pool
// ===========
struct thread_pool {
  pthread_t *threads; // threads array
  int nrOfThreads;    // number of threads the pool created.
  pool_job_t *head;   // linked list first item
  pool_job_t *tail;   // linked list last item
  pthread_mutex_t jobMutex;
  pthread_cond_t signalJob;   // signals threads that there are jobs to be done.
  pthread_cond_t signalNoJob; // signals that there are no threads working.
  size_t activeThreads;       // number of actively working threads.
  size_t aliveThreads;        // number of alive threads.
  bool exit;
};
typedef struct thread_pool thread_pool_t;

// helper function to create jobs
pool_job_t *poolJobCreate(jobFunc func, void *params) {
  pool_job_t *job;

  // null check
  if (func == NULL) {
    return NULL;
  }

  job = malloc(sizeof(pool_job_t));
  job->func = func;
  job->params = params;
  job->next = NULL;

  return job;
}

// helper function to free jobs
void poolJobFree(pool_job_t *job) {
  // null check
  if (job == NULL) {
    return;
  }
  free(job);
}

// helper function for the workers
pool_job_t *poolGetJob(thread_pool_t *tp) {
  pool_job_t *job;

  // null check
  if (tp == NULL) {
    return NULL;
  }

  // get item
  job = tp->head;
  if (job == NULL) {
    return NULL;
  }

  // if next item is null, linked list queue is empty
  if (job->next == NULL) {
    tp->head = NULL;
    tp->tail = NULL;
  } else {
    // otherwise current jobs next, becomes new head.
    tp->head = job->next;
  }

  return job;
}

// does jobs from the job queue
void *poolWorker(void *pool) {
  thread_pool_t *tp = (thread_pool_t *)pool;
  pool_job_t *job;

  while (true) {
    // lock mutex
    pthread_mutex_lock(&(tp->jobMutex));

    // check if linked list queue is empty,
    // if exit is false wait on conditional wakeup when new a job is available.
    // mutex is unlocked automatically by pthread_cond_wait,
    // and then locked again when it gets signaled
    while (tp->head == NULL && tp->exit == false) {
      pthread_cond_wait(&(tp->signalJob), &(tp->jobMutex));
    }

    // stop worker thread if exit is true
    if (tp->exit) {
      break;
    }

    // get job and then unlock mutex
    job = poolGetJob(tp);
    tp->activeThreads++;
    pthread_mutex_unlock(&(tp->jobMutex));

    // if job is not null, do the job and destroy job item.
    if (job != NULL) {
      job->func(job->params);
      poolJobFree(job);
    }

    // when job is done, or no job was found.
    // lock mutex and decrement active working threads,
    // if there are no active threads, send signal if someone is waiting.
    // lastly unlocl mutex.
    pthread_mutex_lock(&(tp->jobMutex));
    tp->activeThreads--;
    if (tp->exit == false && tp->activeThreads == 0 && tp->head == NULL) {
      pthread_cond_signal(&(tp->signalNoJob));
    }
    pthread_mutex_unlock(&(tp->jobMutex));
  }

  // gracefully shutdown thread,
  // unlocks mutex and sends a conditional signal.
  // also decrement aliveThreads.
  tp->aliveThreads--;
  pthread_cond_signal(&(tp->signalNoJob));
  pthread_mutex_unlock(&(tp->jobMutex));
  return NULL;
}

thread_pool_t *poolInit(size_t nrOfThreads) {
  thread_pool_t *tp;
  pthread_t thread;

  // if number of threads is less than 1, early return NULL.
  if (nrOfThreads < 1) {
    return NULL;
  }

  // allocate thread pool struct
  tp = malloc(sizeof(thread_pool_t));
  tp->nrOfThreads = nrOfThreads;
  tp->aliveThreads = nrOfThreads;
  tp->activeThreads = 0;
  tp->exit = false;

  // setup mutex and conditionals
  pthread_mutex_init(&(tp->jobMutex), NULL);
  pthread_cond_init(&(tp->signalJob), NULL);
  pthread_cond_init(&(tp->signalNoJob), NULL);

  // init linked list job queue
  tp->head = NULL;
  tp->tail = NULL;

  // create threads
  tp->threads = malloc(nrOfThreads * sizeof(pthread_t));
  for (int id = 0; id < nrOfThreads; id++) {
    pthread_create(&(tp->threads[id]), NULL, poolWorker, tp);
    // if PRINT == 2, print debug info
    if (PRINT == 2) {
      printf("created thread %d\n", id);
    }
  }

  return tp;
}

// adds a job to the joblist queue
bool poolAddJob(thread_pool_t *tp, jobFunc func, void *params) {
  // TODO:
  pool_job_t *job;

  // null check for the pool
  if (tp == NULL) {
    return false;
  }

  // create new job item
  job = poolJobCreate(func, params);

  // lock mutex
  pthread_mutex_lock(&(tp->jobMutex));
  // add the item to the linked list queue
  // if linked list is empty (head==null),
  // new job item becomes head, and tail becomes head,
  // since the list will only contain one item.
  if (tp->head == NULL) {
    tp->head = job;
    tp->tail = tp->head;
  } else {
    // otherwise just add the job to the tails next,
    // and set the new job to be the new tail.
    tp->tail->next = job;
    tp->tail = job;
  }

  // broadcast a signal saying that there is new work in the job queue.
  pthread_cond_broadcast(&(tp->signalJob));
  pthread_mutex_unlock(&(tp->jobMutex));

  return true;
}

// wait for all active jobs to finish or if exit is true, all threads to be
// killed.
void poolBarrierWait(thread_pool_t *tp) {
  // TODO:
  // null check
  if (tp == NULL) {
    return;
  }

  pthread_mutex_lock(&(tp->jobMutex));
  // wait for all active threads to finish their jobs.
  // or if exit is true, wait for all threads to be killed.
  // also makes sure that the job queue is completly empty before returning.
  // mutex is unlocked automatically by pthread_cond_wait,
  // and then locked again when it gets signaled
  while (true) {
    if ((tp->exit == false && tp->activeThreads != 0) ||
        (tp->exit == true && tp->aliveThreads != 0) ||
        (tp->exit == false && tp->head != NULL && tp->tail != NULL)) {
      pthread_cond_wait(&(tp->signalNoJob), &(tp->jobMutex));
    } else {
      break;
    }
  }
  pthread_mutex_unlock(&(tp->jobMutex));
}

// exit pool as soon as current active threads are done, even if there are more
// jobs in the queue they are discarded.
void poolKill(thread_pool_t *tp) {
  pool_job_t *job1;
  pool_job_t *job2;

  // null check
  if (tp == NULL) {
    return;
  }

  // lock mutex
  // clean up and free any jobs in the queue
  pthread_mutex_lock(&(tp->jobMutex));
  job1 = tp->head;
  while (job1 != NULL) {
    job2 = job1->next;
    poolJobFree(job1);
    job1 = job2;
  }
  // set the exit boolean to true to tell all all threads they should exit.
  // also wake the up all threads by broadcasting a signalJob conditional.
  // lastly unlock the mutex.
  tp->exit = true;
  pthread_cond_broadcast(&(tp->signalJob));
  pthread_mutex_unlock(&(tp->jobMutex));

  // wait for currently active threads in the pool to finish their jobs.
  // and then for all threads to be killed.
  poolBarrierWait(tp);
  for (int id = 0; id < tp->nrOfThreads; id++) {
    if (PRINT == 2) {
      printf("waiting to join thread %d\n", id);
    }
    pthread_join(tp->threads[id], NULL);
  }
  if (PRINT == 2) {
    printf("all threads joined, freeing threads array\n");
  }
  free(tp->threads);

  // free mutex and conditionals.
  pthread_mutex_destroy(&(tp->jobMutex));
  pthread_cond_destroy(&(tp->signalJob));
  pthread_cond_destroy(&(tp->signalNoJob));

  free(tp); // lastly free the pool
}

// ==============================================
//               matrix inversion
// ==============================================

#define MAX_SIZE 4096

typedef double matrix[MAX_SIZE][MAX_SIZE];

int N;            /* matrix size                */
int maxnum;       /* max number of element*/
char *Init;       /* matrix init type   */
matrix A;         /* matrix A           */
matrix I = {0.0}; /* The A inverse matrix, which will be initialized to the
                     identity matrix */

pthread_barrier_t barrier;
struct threadArgs {
  int id;
  int p;
  int col;
  int row;
  double pivalue;
  double multiplier;
};

struct jobArgs {
  int p;
  int col;
  int row;
  double pivalue;
  double multiplier;
};

/* forward declarations */
void find_inverse(void);
void Init_Matrix(void);
void Print_Matrix(matrix M, char name[]);
void Init_Default(void);
void Read_Options(int, char **);
void *child(void *params);
void matrix_to_identity(int p, int col, double pivalue);
void matrix_elimination(int p, int row, int col, double multiplier);
void parallel_find_inverse(thread_pool_t *pool);
void *start_parallel_elimination(void *params);
void *start_parallel_identity(void *params);

static const size_t NR_OF_THREADS = 8;

int main(int argc, char **argv) {
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  printf("Matrix Inverse\n");
  int i, timestart, timeend, iter;

  Init_Default();           /* Init default values      */
  Read_Options(argc, argv); /* Read arguments   */
  Init_Matrix();            /* Init the matrix  */

  // Parallel inversion
  // create pool stuff
  thread_pool_t *pool;
  pool = poolInit(NR_OF_THREADS);
  parallel_find_inverse(pool);
  poolBarrierWait(pool);
  poolKill(pool); // kill pool when inverse is done.

  // print
  if (PRINT == 1) {
    Print_Matrix(A, "End: Input");
    Print_Matrix(I, "Inversed");
  }
}

void matrix_identity_job(void *params) {
  struct jobArgs *args = (struct jobArgs *)params;
  int p = args->p;
  int col = args->col;
  double pivalue = args->pivalue;

  // debug print if PRINT == 2
  if (PRINT == 2) {
    printf("matrix_to_identity: p: %d, col: %d, pivalue: %f\n", p, col,
           pivalue);
  }

  // job
  A[p][col] = A[p][col] / pivalue; /* Division step on A */
  I[p][col] = I[p][col] / pivalue; /* Division step on I */

  free(args);
}

void matrix_elimination_job(void *params) {
  struct jobArgs *args = (struct jobArgs *)params;
  int p = args->p;
  int col = args->col;
  int row = args->row;
  double multiplier = args->multiplier;

  // debug print if PRINT == 2
  if (PRINT == 2) {
    printf("matrix_elimination: p: %d, row: %d, col: %d, multiplier: %f\n", p,
           row, col, multiplier);
  }

  // job
  A[row][col] =
      A[row][col] - A[p][col] * multiplier; /* Elimination step on A */
  I[row][col] =
      I[row][col] - I[p][col] * multiplier; /* Elimination step on I */

  free(args);
}

void parallel_find_inverse(thread_pool_t *pool) {
  int row, col, p; // 'p' stands for pivot (numbered from 0 to N-1)
  double pivalue;  // pivot value

  /* Bringing the matrix A to the identity form */
  for (p = 0; p < N; p++) { /* Outer loop */
    pivalue = A[p][p];
    // add jobs to job queue column wise
    struct jobArgs *identityArgs;
    for (col = 0; col < N; col++) {
      identityArgs = malloc(sizeof(struct jobArgs));
      identityArgs->p = p;
      identityArgs->col = col;
      identityArgs->pivalue = pivalue;
      poolAddJob(pool, matrix_identity_job, identityArgs);
    }
    poolBarrierWait(pool);
    assert(A[p][p] == 1.0);

    // Elimination
    double multiplier;
    for (row = 0; row < N; row++) {
      multiplier = A[row][p];
      if (row != p) // Perform elimination on all except the current pivot row
      {
        // add jobs to job queue column wise
        struct jobArgs *eliminationArgs;
        for (col = 0; col < N; col++) {
          eliminationArgs = malloc(sizeof(struct jobArgs));
          eliminationArgs->p = p;
          eliminationArgs->row = row;
          eliminationArgs->col = col;
          eliminationArgs->multiplier = multiplier;
          poolAddJob(pool, matrix_elimination_job, eliminationArgs);
        }
        poolBarrierWait(pool);
        assert(A[row][p] == 0.0);
      }
    }
  }
}

void Init_Matrix() {
  int row, col;

  // Set the diagonal elements of the inverse matrix to 1.0
  // So that you get an identity matrix to begin with
  for (row = 0; row < N; row++) {
    for (col = 0; col < N; col++) {
      if (row == col)
        I[row][col] = 1.0;
    }
  }

  printf("\nsize      = %dx%d ", N, N);
  printf("\nmaxnum    = %d \n", maxnum);
  printf("Init    = %s \n", Init);
  printf("Initializing matrix...");

  if (strcmp(Init, "rand") == 0) {
    for (row = 0; row < N; row++) {
      for (col = 0; col < N; col++) {
        if (row == col) /* diagonal dominance */
          A[row][col] = (double)(rand() % maxnum) + 5.0;
        else
          A[row][col] = (double)(rand() % maxnum) + 1.0;
      }
    }
  }
  if (strcmp(Init, "fast") == 0) {
    for (row = 0; row < N; row++) {
      for (col = 0; col < N; col++) {
        if (row == col) /* diagonal dominance */
          A[row][col] = 5.0;
        else
          A[row][col] = 2.0;
      }
    }
  }

  printf("done \n\n");
  if (PRINT == 1) {
    // Print_Matrix(A, "Begin: Input");
    // Print_Matrix(I, "Begin: Inverse");
  }
}

void Print_Matrix(matrix M, char name[]) {
  int row, col;

  printf("%s Matrix:\n", name);
  for (row = 0; row < N; row++) {
    for (col = 0; col < N; col++)
      printf(" %5.2f", M[row][col]);
    printf("\n");
  }
  printf("\n\n");
}

void Init_Default() {
  N = 5;
  Init = "fast";
  maxnum = 15.0;
  PRINT = 1;
}

void Read_Options(int argc, char **argv) {
  char *prog;

  prog = *argv;
  while (++argv, --argc > 0)
    if (**argv == '-')
      switch (*++*argv) {
      case 'n':
        --argc;
        N = atoi(*++argv);
        break;
      case 'h':
        printf("\nHELP: try matinv -u \n\n");
        exit(0);
        break;
      case 'u':
        printf("\nUsage: matinv [-n problemsize]\n \
          [-D] show default values \n \
          [-h] help \n \
          [-I init_type] fast/rand \n \
          [-m maxnum] max random no \n \
          [-P print_switch] 0/1 \n");
        exit(0);
        break;
      case 'D':
        printf("\nDefault:  n         = %d ", N);
        printf("\n          Init      = rand");
        printf("\n          maxnum    = 5 ");
        printf("\n          P         = 0 \n\n");
        exit(0);
        break;
      case 'I':
        --argc;
        Init = *++argv;
        break;
      case 'm':
        --argc;
        maxnum = atoi(*++argv);
        break;
      case 'P':
        --argc;
        PRINT = atoi(*++argv);
        break;
      default:
        printf("%s: ignored option: -%s\n", prog, *argv);
        printf("HELP: try %s -u \n\n", prog);
        break;
      }
}
