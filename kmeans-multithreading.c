#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_POINTS 4096
#define MAX_CLUSTERS 32
#define NR_THREADS 8

typedef struct {
  int id;
  int start;
  int end;
  int k_start;
  int k_end;
  pthread_mutex_t *mutex;
} threadArgs;

typedef struct {
  pthread_t threads[NR_THREADS];
  threadArgs tArgs[NR_THREADS];
} threadHandler;

typedef struct point {
  float x;     // The x-coordinate of the point
  float y;     // The y-coordinate of the point
  int cluster; // The cluster that the point belongs to
} point;

int N;                       // number of entries in the data
int k;                       // number of centroids
point data[MAX_POINTS];      // Data coordinates
point cluster[MAX_CLUSTERS]; // The coordinates of each cluster center (also
                             // called centroid)
threadHandler thandler;
pthread_mutex_t mutex;
pthread_barrier_t barrier;
const bool True = true;
const bool False = false;

// THREADHANDLER FUNCTIONS

void joinThreads(threadHandler *tHandler) {
  for (int i = 0; i < NR_THREADS; i++) {
    pthread_join(tHandler->threads[i], NULL);
  }
}

void initThreadArgs(threadHandler *tHandler, pthread_mutex_t *mutex,
                    int *intervals, int *k_intervals) {
  for (int i = 0; i < NR_THREADS; i++) {
    tHandler->tArgs[i].id = i;
    tHandler->tArgs[i].mutex = mutex;
    tHandler->tArgs[i].start = intervals[i];
    tHandler->tArgs[i].end = intervals[i + 1];
    tHandler->tArgs[i].k_start = k_intervals[i];
    tHandler->tArgs[i].k_end = k_intervals[i + 1];
  }
}

void assignThreadIntervals(int *intervals, int *k_intervals, int N, int k) {
  int i;
  int interval = N / NR_THREADS;
  int size = NR_THREADS * 2;
  for (i = 0; i < NR_THREADS; i++) {
    intervals[i] = interval * i;
    if (i == NR_THREADS - 1) {
      intervals[i + 1] = N;
    } else {
      intervals[i + 1] = (interval * (i + 1));
    }
  }

  if (k <= NR_THREADS) {
    for (int i = 0; i < k; i++) {
      k_intervals[i] = i;
      k_intervals[i + 1] = i + 1;
    }
  } else {
    int const k_interval = k / NR_THREADS;
    for (i = 0; i < NR_THREADS; i++) {
      k_intervals[i] = k_interval * i;
      if (i == NR_THREADS - 1) {
        k_intervals[i + 1] = k;
      } else {
        k_intervals[i + 1] = (k_interval * (i + 1));
      }
    }
  }
}

void printIntervals(threadHandler *thandler) {
  for (int i = 0; i < NR_THREADS; i++) {
    printf("N intervals start: %d, end: %d\n", thandler->tArgs[i].start,
           thandler->tArgs[i].end);
  }
  for (int i = 0; i < NR_THREADS; i++) {
    printf("K intervals start: %d, end: %d\n", thandler->tArgs[i].k_start,
           thandler->tArgs[i].k_end);
  }
}

// KMEANS

void read_data() {
  N = 1797;
  k = 9;
  FILE *fp = fopen("kmeans-data/kmeans-data.txt", "r");
  if (fp == NULL) {
    perror("Cannot open the file");
    exit(EXIT_FAILURE);
  }

  // Initialize points from the data file
  for (int i = 0; i < N; i++) {
    fscanf(fp, "%f %f", &data[i].x, &data[i].y);
    data[i].cluster = -1; // Initialize the cluster number to -1
  }
  printf("Read the problem data!\n");
  // Initialize centroids randomly
  srand(0); // Setting 0 as the random number generation seed
  for (int i = 0; i < k; i++) {
    int r = rand() % N;
    cluster[i].x = data[r].x;
    cluster[i].y = data[r].y;
  }
  fclose(fp);
}

int get_closest_centroid_t(int i, int k_start, int k_end) {
  /* find the nearest centroid */
  int nearest_cluster = -1;
  double xdist, ydist, dist, min_dist;
  min_dist = dist = INT_MAX;
  for (int c = k_start; c < k_end; c++) { // For each centroid
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

bool assign_clusters_to_points_t(threadArgs *args) {
  int old_cluster = -1, new_cluster = -1;
  bool something_changed = false;
  for (int i = args->start; i < args->end; i++) { // For each data point
    old_cluster = data[i].cluster;
    new_cluster = get_closest_centroid_t(i, args->k_start, args->k_end);
    data[i].cluster = new_cluster; // Assign a cluster to the point i
    if (old_cluster != new_cluster) {
      something_changed = true;
    }
  }
  return something_changed;
}

void update_cluster_centers_t(threadArgs *args) {
  /* Update the cluster centers */
  int c;
  int count[MAX_CLUSTERS]; // Array to keep track of the number of points in
                           // each cluster
  point temp[MAX_CLUSTERS];
  pthread_barrier_wait(&barrier);
  for (int i = args->start; i < args->end; i++) {
    c = data[i].cluster;
    count[c]++;
    temp[c].x += data[i].x;
    temp[c].y += data[i].y;
  }
  pthread_barrier_wait(&barrier);
  if (args->k_start != -1 &&
      args->k_end !=
          -1) // If k < number of threads and this thread hasn't been assigned a
              // cluster/clusters it will stop here and finish
  {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < k; i++) {
      cluster[i].x = temp[i].x / count[i];
      cluster[i].y = temp[i].y / count[i];
    }
    pthread_mutex_unlock(&mutex);
  }
}

void *threadWorker(void *args_t) {
  threadArgs *args = (threadArgs *)args_t;
  bool something_changed = true;
  while (something_changed) {
    something_changed = assign_clusters_to_points_t(args);
    pthread_barrier_wait(&barrier);
    update_cluster_centers_t(args);
    pthread_barrier_wait(&barrier);
  }
  pthread_exit(NULL);
}

void kmeans(int k) {
  int iter = 0;
  int *k_intervals = (int *)malloc(sizeof(k_intervals) * (NR_THREADS * 2));
  int *intervals = (int *)malloc(sizeof(intervals) * (NR_THREADS * 2));
  assignThreadIntervals(intervals, k_intervals, N, k);
  initThreadArgs(&thandler, &mutex, intervals, k_intervals);
  pthread_barrier_init(&barrier, NULL, NR_THREADS);
  for (int i = 0; i < NR_THREADS; i++) {
    pthread_create(&thandler.threads[i], NULL, &threadWorker,
                   &thandler.tArgs[i]);
  }
  joinThreads(&thandler);
  printf("Number of iterations taken = %d\n", iter);
  printf("Computed cluster numbers successfully!\n");
}

void write_results() {
  FILE *fp = fopen("kmeans-results.txt", "w");
  if (fp == NULL) {
    perror("Cannot open the file");
    exit(EXIT_FAILURE);
  } else {
    for (int i = 0; i < N; i++) {
      fprintf(fp, "%.2f %.2f %d\n", data[i].x, data[i].y, data[i].cluster);
    }
  }
  printf("Wrote the results to a file!\n");
}

int main() {
  read_data();
  kmeans(k);
  write_results();
}