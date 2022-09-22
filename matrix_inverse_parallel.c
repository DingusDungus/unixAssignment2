/***************************************************************************
 *
 * Sequential version of Matrix Inverse
 * An adapted version of the code by Hkan Grahn
 ***************************************************************************/

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
