#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// if (PRINT == 1), prints matrixes.
// if (PRINT == 2), prints matrixes + debug info.
int PRINT; /* print switch               */

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
//               kmeans algorithm
// ==============================================

#define MAX_POINTS 4096
#define MAX_CLUSTERS 32

typedef struct point {
  float x;     // The x-coordinate of the point
  float y;     // The y-coordinate of the point
  int cluster; // The cluster that the point belongs to
} point;

char *fileName = NULL;       // name of input file
int N;                       // number of entries in the data
int k = 0;                   // number of centroids
point data[MAX_POINTS];      // Data coordinates
point cluster[MAX_CLUSTERS]; // The coordinates of each cluster center (also
                             // called centroid)

void read_data() {
  // init values if nothing was provided via the command-line flags.
  if (fileName == NULL) {
    fileName = "kmeans-data/kmeans-data.txt";
    printf("No filename provided, reading default file: %s\n", fileName);
  }
  if (k == 0) {
    k = 9;
    printf("No k value provided, set k to default: %d\n", k);
    printf("================================");
    printf(" Kmeans ");
    printf("================================\n");
  }
  // open file
  FILE *fp = fopen(fileName, "r");
  if (fp == NULL) {
    perror("Cannot open the file");
    exit(EXIT_FAILURE);
  }
  // Initialize points from the data file
  float temp;
  int i = 0;
  char ch;
  while (true) {
    ch = fscanf(fp, "%f %f", &data[i].x, &data[i].y);
    if (ch == EOF) {
      break;
    }
    data[i].cluster = -1; // Initialize the cluster number to -1
    i++;
  }
  N = i; // set number of entries in data.
  printf("Read the problem data!\n");
  printf("number of entries: %d\n", N);
  // Initialize centroids randomly
  srand(0); // Setting 0 as the random number generation seed
  for (int i = 0; i < k; i++) {
    int r = rand() % N;
    cluster[i].x = data[r].x;
    cluster[i].y = data[r].y;
  }
  fclose(fp);
}

int get_closest_centroid(int i, int k) {
  /* find the nearest centroid */
  int nearest_cluster = -1;
  double xdist, ydist, dist, min_dist;
  min_dist = dist = INT_MAX;
  for (int c = 0; c < k; c++) { // For each centroid
    // Calculate the square of the Euclidean distance between that centroid and
    // the point
    xdist = data[i].x - cluster[c].x;
    ydist = data[i].y - cluster[c].y;
    dist = xdist * xdist + ydist * ydist; // The square of Euclidean distance
    // printf("%.2lf \n", dist);
    if (dist <= min_dist) {
      min_dist = dist;
      nearest_cluster = c;
    }
  }
  // printf("-----------\n");
  return nearest_cluster;
}

// TODO: add start and end instead of index.
struct assignJobArgs {
  int index;               // what data point index should be worked on.
  bool *something_changed; // something_changed bool pointer supplied by creator
                           // of job to be shared by all jobs of this type.
  pthread_mutex_t *assignMutex; // mutex supplied by creator of job to be shared
                                // by all jobs of this type.
};

// job function to be added to queue by assign_clusters_to_points
void assign_clusters_job(void *params) {
  // TODO: add for loop so function can work on a range and not just one index.
  struct assignJobArgs *args = (struct assignJobArgs *)params;
  int id = args->index;
  pthread_mutex_t *assignMutex = args->assignMutex;
  bool *something_changed = args->something_changed;

  int old_cluster = -1, new_cluster = -1;
  old_cluster = data[id].cluster;
  new_cluster = get_closest_centroid(id, k);
  data[id].cluster = new_cluster; // Assign a cluster to the point i
  if (old_cluster != new_cluster) {
    pthread_mutex_lock(assignMutex);
    *something_changed = true;
    pthread_mutex_unlock(assignMutex);
  }
  free(params);
}

// for each thread, check that their something_changed bool is true, only
// if it's true, we want to change to main threads functions something_changed
// variable to true. And it main thred funcs var is already true, we dont do
// anything in the thread. Needs a mutex lock on the something_changed var from
// the main thread func.
bool assign_clusters_to_points(thread_pool_t *pool) {
  bool something_changed = false;
  struct assignJobArgs *assignJobArgs;
  pthread_mutex_t assignMutex;
  pthread_mutex_init(&assignMutex, NULL);
  // TODO: split work into jobs, something like N / nrOfThreads.
  // but make sure to handle the remainder so work is not missed.
  for (int i = 0; i < N; i++) { // For each data point
    assignJobArgs = malloc(sizeof(struct assignJobArgs));
    assignJobArgs->index = i;
    assignJobArgs->something_changed = &something_changed;
    assignJobArgs->assignMutex = &assignMutex;
    poolAddJob(pool, assign_clusters_job, assignJobArgs);
    // printf("added job %d\n", i);
  }
  poolBarrierWait(pool);
  return something_changed;
}

// TODO: add start and end instead of index.
struct updateJobArgs {
  int index;   // the index
  int c;       // the cluster that the point belongs to.
  int *count;  // Array to keep track of the number of points in
               // each cluster
  point *temp; // temp array
};

// job function to be added to queue by update_cluster_centers
void update_clusters_first_job(void *params) {
  // TODO: add for loop so function can work on a range and not just one index.
  struct updateJobArgs *args = (struct updateJobArgs *)params;
  int c = args->c;
  int index = args->index;

  args->count[c]++;
  args->temp[c].x += data[index].x;
  args->temp[c].y += data[index].y;
  free(params);
}

// job function to be added to queue by update_cluster_centers
void update_clusters_second_job(void *params) {
  struct updateJobArgs *args = (struct updateJobArgs *)params;
  int index = args->index;
  cluster[index].x = args->temp[index].x / args->count[index];
  cluster[index].y = args->temp[index].y / args->count[index];
  free(params);
}

void update_cluster_centers(thread_pool_t *pool) {
  // TODO: create jobs for each (0-N) that do second loop (0-k) as their job
  /* Update the cluster centers */
  int c;
  int *count;
  count = calloc(1, sizeof(int[MAX_CLUSTERS]));
  point *temp;
  temp = calloc(1, sizeof(point[MAX_CLUSTERS]));

  // TODO: split work into jobs, something like N / nrOfThreads.
  // but make sure to handle the remainder so work is not missed.
  struct updateJobArgs *updateJobArgs;
  for (int i = 0; i < N; i++) {
    updateJobArgs = malloc(sizeof(struct updateJobArgs));
    updateJobArgs->index = i;
    updateJobArgs->c = data[i].cluster;
    updateJobArgs->count = count;
    updateJobArgs->temp = temp;
    poolAddJob(pool, update_clusters_first_job, updateJobArgs);
  }
  poolBarrierWait(pool);
  // maybe need to wait inbetween steps/loops
  for (int i = 0; i < k; i++) {
    updateJobArgs = malloc(sizeof(struct updateJobArgs));
    updateJobArgs->index = i;
    updateJobArgs->count = count;
    updateJobArgs->temp = temp;
    poolAddJob(pool, update_clusters_second_job, updateJobArgs);
  }
  poolBarrierWait(pool);
  // free memory
  free(count);
  free(temp);
}

void kmeans(int k, thread_pool_t *pool) {
  bool somechange;
  int iter = 0;
  do {
    iter++; // Keep track of number of iterations
    somechange = assign_clusters_to_points(pool);
    update_cluster_centers(pool);
    poolBarrierWait(pool);
  } while (somechange);
  printf("Number of iterations taken = %d\n", iter);
  printf("Computed cluster numbers successfully!\n");
}

void write_results() {
  fileName = "kmeans-results.txt";
  FILE *fp = fopen(fileName, "w");
  if (fp == NULL) {
    perror("Cannot open the file");
    exit(EXIT_FAILURE);
  } else {
    for (int i = 0; i < N; i++) {
      fprintf(fp, "%.2f %.2f %d\n", data[i].x, data[i].y, data[i].cluster);
    }
  }
  printf("Wrote the results to file: %s\n", fileName);
}

void Read_Options(int argc, char **argv) {
  char *prog;

  prog = *argv;
  while (++argv, --argc > 0)
    if (**argv == '-')
      switch (*++*argv) {
      case 'k':
        --argc;
        k = atoi(*++argv);
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
      case 'f':
        --argc;
        fileName = *++argv;
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

static const size_t NR_OF_THREADS = 8;

int main(int argc, char **argv) {
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  Read_Options(argc, argv);
  read_data();
  // create pool stuff
  thread_pool_t *pool;
  pool = poolInit(NR_OF_THREADS);
  kmeans(k, pool);       // run kmeans
  poolBarrierWait(pool); // wait for jobs to be done and threads to finish.
  poolKill(pool);        // kill pool when inverse is done.
  write_results();
}
