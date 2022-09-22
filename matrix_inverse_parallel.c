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
  pool_job_t *head; // linked list first item
  pool_job_t *tail; // linked list last item
  pthread_mutex_t jobMutex;
  pthread_cond_t signalJob;   // signals threads that there are jobs to be done.
  pthread_cond_t signalNoJob; // signals that there are no threads working.
  size_t activeThreads;       // number of actively working threads.
  size_t aliveThreads;        // number of alive threads.
  bool exit;
};
typedef struct thread_pool thread_pool_t;

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

void poolJobFree(pool_job_t *job) {
  // null check
  if (job == NULL) {
    return;
  }
  free(job);
}

pool_job_t *poolGetJob(thread_pool_t *tp) {
  pool_job_t *job;

  // null check
  if (tp == NULL) {
    return NULL;
  }

  // get item
  job = tp->head;
  // null check
  if (job == NULL) {
    return NULL;
  }

  // if next item is null, linked list is empty
  if (job->next == NULL) {
    tp->head = NULL;
    tp->tail = NULL;
  } else {
    // first item in linked list becomse current items next
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
  for (size_t i = 0; i < nrOfThreads; i++) {
    pthread_create(&thread, NULL, poolWorker, tp);
    pthread_detach(thread);
  }

  return tp;
}

// exit pool as soon as current active threads are done, even if there is more
// jobs in the queue
void poolKill(thread_pool_t *tp) {
  pool_job_t *jobOne;
  pool_job_t *jobTwo;

  // null check
  if (tp == NULL) {
    return;
  }

  pthread_mutex_lock(&(tp->jobMutex));
  jobOne = tp->head;
  // TODO: kill/destroy function + the rest.
}

// ==============================================
//               matrix inversion
// ==============================================

#define MAX_SIZE 4096

typedef double matrix[MAX_SIZE][MAX_SIZE];

int N;            /* matrix size                */
int maxnum;       /* max number of element*/
char *Init;       /* matrix init type   */
int PRINT;        /* print switch               */
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

/* forward declarations */
void find_inverse(void);
void Init_Matrix(void);
void Print_Matrix(matrix M, char name[]);
void Init_Default(void);
void Read_Options(int, char **);
void *child(void *params);
void matrix_to_identity(int p, int col, double pivalue);
void matrix_elimination(int p, int row, int col, double multiplier);
void parallel_find_inverse();
void *start_parallel_elimination(void *params);
void *start_parallel_identity(void *params);

int main(int argc, char **argv) {
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  printf("Matrix Inverse\n");
  int i, timestart, timeend, iter;

  Init_Default();           /* Init default values      */
  Read_Options(argc, argv); /* Read arguments   */
  Init_Matrix();            /* Init the matrix  */

  // Sequential
  // printf("Starting Sequential\n");
  // find_inverse();
  // printf("Sequential done\n");

  // Parallel
  parallel_find_inverse();

  // print
  if (PRINT == 1) {
    Print_Matrix(A, "End: Input");
    Print_Matrix(I, "Inversed");
  }
}

void *child(void *params) {
  struct threadArgs *args = (struct threadArgs *)params;
  int id = args->id;
  int p = args->p;
  int col = args->col;
  int row = args->row;
  double pivalue = args->pivalue;
  double multiplier = args->multiplier;
  // printf("Proccess %d starting work...\n", id);
  matrix_to_identity(p, id, pivalue);
  // pthread_barrier_wait(&barrier);
  free(args);
  // printf("Proccess %d done and freed.\n", id);
  return NULL;
}

void *start_parallel_identity(void *params) {
  struct threadArgs *args = (struct threadArgs *)params;
  int id = args->id;
  int p = args->p;
  int col = args->col;
  int row = args->row;
  double pivalue = args->pivalue;
  double multiplier = args->multiplier;
  // printf("Proccess %d starting work...\n", id);
  matrix_to_identity(p, id, pivalue);
  pthread_barrier_wait(&barrier);
  free(args);
  // printf("Proccess %d done and freed.\n", id);
  return NULL;
}

void *start_parallel_elimination(void *params) {
  struct threadArgs *args = (struct threadArgs *)params;
  int id = args->id;
  int p = args->p;
  int col = args->col;
  int row = args->row;
  double pivalue = args->pivalue;
  double multiplier = args->multiplier;
  // printf("Proccess %d starting work...\n", id);
  matrix_elimination(p, row, col, multiplier);
  pthread_barrier_wait(&barrier);
  free(args);
  // printf("Proccess %d done and freed.\n", id);
  return NULL;
}

void start_children(void *workerFunc, struct threadArgs *threadArgs) {
  // printf("start: start_children\n");
  struct threadArgs *args;
  pthread_t *children;
  int id = 0;
  pthread_barrier_init(&barrier, NULL, N);

  children = malloc(N * sizeof(pthread_t));
  for (id = 0; id < N; id++) {
    args = malloc(sizeof(struct threadArgs));
    args->id = id;
    args->p = threadArgs->p;
    args->col = id;
    args->row = threadArgs->row;
    args->pivalue = threadArgs->pivalue;
    args->multiplier = threadArgs->multiplier;
    pthread_create(&(children[id]), NULL, workerFunc, (void *)args);
    // printf("created thread(col) %d, p: %d, row: %d\n", id, args->p,
    // args->row);
  }
  for (id = 0; id < N; id++) {
    pthread_join(children[id], NULL);
    // printf("joined thread(col) %d, p: %d, row: %d\n", id, args->p,
    // threadArgs->row);
  }
  free(children);
  free(threadArgs);
  // printf("end: start_children\n");
}

void parallel_find_inverse() {
  int row, p;     // 'p' stands for pivot (numbered from 0 to N-1)
  double pivalue; // pivot value

  /* Bringing the matrix A to the identity form */
  for (p = 0; p < N; p++) { /* Outer loop */
    pivalue = A[p][p];
    // create thread args and start worker threads with matrix_to_identity
    // function.
    struct threadArgs *identityArgs;
    identityArgs = malloc(sizeof(struct threadArgs));
    identityArgs->pivalue = pivalue;
    identityArgs->p = p;
    // printf("call start_children identity\n");
    start_children(start_parallel_identity, identityArgs);
    // assert(A[p][p] == 1.0);

    // Elimination
    double multiplier;
    for (row = 0; row < N; row++) {
      multiplier = A[row][p];
      if (row != p) // Perform elimination on all except the current pivot row
      {
        // create thread args and start worker threads with matrix_elimination
        // function.
        struct threadArgs *eliminationArgs;
        eliminationArgs = malloc(sizeof(struct threadArgs));
        eliminationArgs->multiplier = multiplier;
        eliminationArgs->p = p;
        eliminationArgs->row = row;
        // printf("call start_children elimination, row: %d\n", row);
        start_children(start_parallel_elimination, identityArgs);
        // assert(A[row][p] == 0.0);
      } else {
        // printf("elimination skipping row: %d\n", row);
      }
    }
  }
}

void matrix_to_identity(int p, int col, double pivalue) {
  // printf("matrix_to_identity: p: %d, col: %d, pivalue: %f\n", p, col,
  // pivalue);
  A[p][col] = A[p][col] / pivalue; /* Division step on A */
  I[p][col] = I[p][col] / pivalue; /* Division step on I */
}

void matrix_elimination(int p, int row, int col, double multiplier) {
  // printf("matrix_elimination: p: %d, row: %d, col: %d, multiplier: %f\n", p,
  // row, col, multiplier);
  A[row][col] =
      A[row][col] - A[p][col] * multiplier; /* Elimination step on A */
  I[row][col] =
      I[row][col] - I[p][col] * multiplier; /* Elimination step on I */
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
