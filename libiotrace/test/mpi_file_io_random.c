#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mpi.h"
#include "omp.h"
#include <argp.h>
#include <time.h>

// #define WANT_OPENMP
// #define WANT_CLEAR_CACHES
#ifdef WANT_CLEAR_CACHES
#  define SIZE_L3_CACHE    (8 * 1024 * 1024) // 8 MB
#  define CLEAR_CACHES_PER_ITERATIONS 1
#endif

#ifdef WANT_OPENMP
#  ifndef WANT_THREADS
#    define WANT_THREADS
#  endif /* WANT_THREADS */
#endif /* WANT_OPENMP */

#ifdef WANT_THREADS
#  define WANT_MPI_THREAD_LEVEL  MPI_THREAD_MULTIPLE // Either one of MPI_THREAD_MULTIPLE (highest), MPI_THREAD_FUNNELED, MPI_THREAD_SINGLE
#endif

// For Debug, uncomment
//#define DEBUG(x) x
//#define DEBUG(x)

#define ERROR_FATAL(s,val)   do { \
        char _mpi_error_string[MPI_MAX_ERROR_STRING]; \
        int _len = MPI_MAX_ERROR_STRING; \
        MPI_Error_string((val), _mpi_error_string, &_len); \
        fprintf(stderr, "ERROR (%s:%d) in %s errval:%d (in case of POSIX call:'%s', in case of MPI:'%s')\n", \
                __FILE__, __LINE__, (s), val, strerror(val), _mpi_error_string); \
        exit(val); \
    } while(0)

#define MPI_CHECK(x)  do { \
        int _ret = x; \
        if (_ret != MPI_SUCCESS) { \
            ERROR_FATAL("MPI", _ret);\
        } \
    } while(0)


#ifdef WANT_CLEAR_CACHES

/**
 * Clears the Data caches, aka it reads and modifies data as large the L3 cache
 */
void clear_dcache(void) {
    static char * buffer = NULL;
    if (NULL == buffer) {
        buffer = malloc(SIZE_L3_CACHE);
        if (NULL == buffer)
            ERROR_FATAL("malloc", ENOMEM);
    }
    buffer[0]++;
    for (int i = 1; i < SIZE_L3_CACHE; i++) {
        buffer[i] += buffer[i-1];
    }
}

#define NOP1      __asm__ __volatile__ ("nop\n\t");
#define NOP2     __asm__ __volatile__ ("mov %eax, %eax\n\t");
#define NOP3     __asm__ __volatile__ ("rolq $0x20, %rax\n\t");
#define NOP           NOP1 NOP2 NOP3
#define NOP10         NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP
#define NOP100        NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10
#define NOP1000  do { NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 NOP100 } while(0)
/**
 * Clears the Instruction cache (icache), by jumping about some code.
 */
void clear_icache(void) {
    NOP1000;
}
#endif /* WANT_CLEAR_CACHES */


int randBeetween(int min, int max) {
  return (rand() % (max - min + 1)) + min;
}

// ARGS PARSER

const char *argp_program_version ="posix_file_io_random 1.0";
const char *argp_program_bug_address = "@hs-esslingen.de";
struct arguments {            
  int verbose;              
  int iterations;              
  int timeBetweenFileCreateMin;              
  int timeBetweenFileCreateMax;                          
  int threadMin;              
  int threadMax;              
  int fileCountMin;              
  int fileCountMax;              
  int readMin;              
  int readMax;              
  int writeMin;              
  int writeMax;              
};

static char doc[] ="posix_file_io_random -- A program to generate Random FILE IO.";

static struct argp_option options[] = {
  {"verbose"                  , 'v', 0            , OPTION_ARG_OPTIONAL, "Produce verbose output (Default: false)", 0},
  {"iterations"               , 'i', "iterations" , OPTION_ARG_OPTIONAL, "Number of Iterations (Default: INT_MAX)", 0},
  {"time-between-file-create-min" , 'c', "timeBetweenFileCreateMin"       , OPTION_ARG_OPTIONAL, "Min Time to wait beteween open Files (Default: 0)", 0},
  {"time-between-file-create-max" , 'C', "timeBetweenFileCreateMax"       , OPTION_ARG_OPTIONAL, "Max Time to wait beteween open Files (Default: 0)", 0},

  {"thread-min"               , 't', "threadMin", OPTION_ARG_OPTIONAL, "Min thread count per process (Default: 10)", 0},
  {"thread-max"               , 'T', "threadMax", OPTION_ARG_OPTIONAL, "Max thread count per process (Default: 10)", 0},
  
  {"file-count-min"           , 'f', "fileCountMin", OPTION_ARG_OPTIONAL, "Min thread count per process (Default: 10", 0},
  {"file-count-max"           , 'F', "fileCountMax", OPTION_ARG_OPTIONAL, "Max thread count per process (Default: 10)", 0},

  {"read-min"                 , 'r', "readMin", OPTION_ARG_OPTIONAL, "Min read count per File in byte (Default: 10)", 0},
  {"read-max"                 , 'R', "readMax", OPTION_ARG_OPTIONAL, "Max read count per File in byte (Default: 10)", 0},
  
  {"write-min"                , 'w', "writeMin", OPTION_ARG_OPTIONAL, "Min write count per File in byte (Default: 10)", 0},
  {"write-max"                , 'W', "writeMax", OPTION_ARG_OPTIONAL, "Max write count per File in byte (Default: 10)", 0},
  {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'v':
      arguments->verbose = 1;
      break;
    case 'i':
      arguments->iterations = atoi(arg);
      break;
    case 'c':
      arguments->timeBetweenFileCreateMin = atoi(arg);
      break;
    case 'C':
      arguments->timeBetweenFileCreateMax = atoi(arg);
      break;
    case 't':
      arguments->threadMin = atoi(arg);
      break;
    case 'T':
      arguments->threadMax = atoi(arg);
      break;
    case 'f':
      arguments->fileCountMin = atoi(arg);
      break;
    case 'F':
      arguments->fileCountMax = atoi(arg);
      break;
    case 'r':
      arguments->readMin = atoi(arg);
      break;
    case 'R':
      arguments->readMax = atoi(arg);
      break;
    case 'w':
      arguments->writeMin = atoi(arg);
      break;
    case 'W':
      arguments->writeMax = atoi(arg);
      break;
    case ARGP_KEY_ARG:
      break;
    case ARGP_KEY_END:
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static char args_doc[] = "";

static struct argp argp = {options, parse_opt, args_doc, doc,0,0,0};


void parse_args(int argc, char * argv[], struct arguments* arguments) {
    /* Set argument defaults */
    arguments->verbose = 0;
    arguments->iterations = INT_MAX;
    arguments->timeBetweenFileCreateMin = 0;
    arguments->timeBetweenFileCreateMax = 0;
    arguments->threadMin = 10;
    arguments->threadMax = 10;
    arguments->fileCountMin = 10;
    arguments->fileCountMax = 10;
    arguments->readMin = 10;
    arguments->readMax = 10;
    arguments->writeMin = 10;
    arguments->writeMax = 10;

    argp_parse (&argp, argc, argv, 0, 0, arguments);

    if (arguments->threadMin > arguments->threadMax) {
      arguments->threadMin = arguments->threadMax;
    }
    if (arguments->fileCountMin > arguments->fileCountMax) {
      arguments->fileCountMin = arguments->fileCountMax;
    }
    if (arguments->readMin > arguments->readMax) {
      arguments->readMin = arguments->readMax;
    }
    if (arguments->writeMin > arguments->writeMax) {
      arguments->writeMin = arguments->writeMax;
    }
    if (arguments->timeBetweenFileCreateMin > arguments->timeBetweenFileCreateMax) {
      arguments->timeBetweenFileCreateMin = arguments->timeBetweenFileCreateMax;
    }
}

// END ARGS PARSER

char *static_write_buffer="abcdefghiklmnopqrstuvxyz012345789|abcdefghiklmnopqrstuvxyz012345789|abcdefghiklmnopqrstuvxyz012345789|abcdefghiklmnopqrstuvxyz012345789|abcdefghiklmnopqrstuvxyz012345789|abcdefghiklmnopqrstuvxyz012345789|abcdefghiklmnopqrstuvxyz012345789|abcdefghiklmnopqrstuvxyz012345789|abcdefghiklmnopqrstuvxyz012345789|abcdefghiklmnopqrstuvxyz012345789|";

typedef struct FileState {            
  FILE* file;              
  char filename[80];              
  long timeOpen;
  long timeLastRead;
  long timeLastWrite;

  int iterationsWrite;
  int iterationsRead;
  long lastPositionInFile;                           
} FileState;

int main (int argc, char * argv[]) {
    int comm_size;
    int comm_rank;
    int thread_num = 1;
    int thread_me = 0;
    int time_start = 0;

    struct arguments arguments;

    parse_args(argc, argv, &arguments);

#   ifdef WANT_THREADS
    int mpi_thread_level = WANT_MPI_THREAD_LEVEL;
    int mpi_thread_level_supported;
    MPI_CHECK(MPI_Init_thread(&argc, &argv, mpi_thread_level, &mpi_thread_level_supported));
    if (mpi_thread_level != mpi_thread_level_supported) {
        ERROR_FATAL("MPI Thread-level not supported", EINVAL);
    }
#   else
    MPI_CHECK(MPI_Init(&argc, &argv));
#   endif /* WANT_THREADS */

  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  if (arguments.verbose) {
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"verbose", arguments.verbose);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"iterations", arguments.iterations);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"timeBetweenFileCreateMin", arguments.timeBetweenFileCreateMin);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"timeBetweenFileCreateMax", arguments.timeBetweenFileCreateMax);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"threadMin", arguments.threadMin);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"threadMax", arguments.threadMax);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"fileCountMin", arguments.fileCountMin);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"fileCountMax", arguments.fileCountMax);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"readMin", arguments.readMin);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"readMax", arguments.readMax);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"writeMin", arguments.writeMin);
       printf("[%d] Paramter: %25s = %d\n", comm_rank,"writeMax", arguments.writeMax);
  }

# ifdef WANT_OPENMP
  srand(time(NULL) + comm_rank);
  int randomThreadcountForProcess =  randBeetween(arguments.threadMin ,arguments.threadMax);
  if (arguments.verbose) {
    printf("[%d] Paramter: %25s = %d\n", comm_rank, "randomThreadcountForProcess", randomThreadcountForProcess);
  }
  omp_set_num_threads(randomThreadcountForProcess);
# endif

# ifdef WANT_OPENMP
  #pragma omp parallel private(thread_me, thread_num)
# endif
  {
#   ifdef WANT_OPENMP
    {
      thread_num = omp_get_num_threads();
      thread_me = omp_get_thread_num();
    }
#   endif

    time_start = time(NULL);

    if (arguments.verbose) {
      printf("[%2d | %2d/%2d ] Paramter: %25s = %d\n", comm_rank, thread_me, thread_num, "time", time_start);
    }

    int fileCount = randBeetween(arguments.fileCountMin ,arguments.fileCountMax);
    FileState *fs = calloc(fileCount, sizeof(FileState));
    int opendFiles = 0;
    long nextTimeOpenFile = time(NULL) + randBeetween(arguments.timeBetweenFileCreateMin ,arguments.timeBetweenFileCreateMax);
    char isDone = 0;
    long timeLastLog = time(NULL);
    do {
      if(timeLastLog + 10 < time(NULL)) {
        printf("[%2d | %2d/%2d ] INFO: opendFiles: %d/%d\n", comm_rank, thread_me, thread_num, opendFiles, fileCount);
        timeLastLog = time(NULL);
      }

      //Open Files after time;
      if (opendFiles < fileCount && time(NULL) > nextTimeOpenFile) {
        FileState *filestate = &fs[opendFiles];
        int timespan = randBeetween(arguments.timeBetweenFileCreateMin ,arguments.timeBetweenFileCreateMax);
        nextTimeOpenFile += timespan;
        sprintf(filestate->filename, "mpi_file_random_%02d_%02d_%04d.txt", comm_rank, thread_me, opendFiles);
        
        printf("[%2d | %2d/%2d ] INFO: STACK HERE 1: %s\n", comm_rank, thread_me, thread_num, filestate->filename);
        
        filestate->file = fopen(filestate->filename, "w+");
        if (filestate->file == NULL) {
          printf("The file is not opened. The program will now exit.");
          exit(0);
        }
  
        printf("[%2d | %2d/%2d ] INFO: STACK HERE 2\n", comm_rank, thread_me, thread_num);

        
        filestate->timeOpen = time(NULL);

        if (arguments.verbose) {
          printf("(%ld) [%2d | %2d ] %d Paramter: %25s = %d\n", time(NULL), comm_rank, thread_me, opendFiles+1, "nextTimeOpenFile in", timespan);
          printf("(%ld) [%2d | %2d ] Open File %d: %s\n",time(NULL), comm_rank, thread_me, opendFiles, filestate->filename);
        }
        opendFiles++;      
      }


      if (opendFiles > 0) {
        int randomFilePos = randBeetween(0, (opendFiles-1));
        FileState *filestate = &fs[randomFilePos];
        if(filestate->timeOpen != 0 && filestate->file != NULL) {
          
          // write n bytes in file
          if (randBeetween(0, 100) % 2 == 0 && filestate->iterationsWrite < arguments.iterations && time(NULL) >= (filestate->timeLastWrite + 1)) {
            int bytes_write = randBeetween(arguments.writeMin, arguments.writeMax);

            fseek(filestate->file, filestate->lastPositionInFile, SEEK_SET); 
            fwrite(static_write_buffer, bytes_write, 1, filestate->file); 

            if (arguments.verbose) {
              printf("(%ld) [%2d | %2d ] Write %d Byte in File: %s\n", time(NULL), comm_rank, thread_me, bytes_write, filestate->filename);
            }
            filestate->lastPositionInFile += bytes_write;
            filestate->iterationsWrite++;
            filestate->timeLastWrite = time(NULL);
          }

          // read n bytes from file
          if (randBeetween(0, 100) % 2 == 0 && filestate->iterationsRead < arguments.iterations && time(NULL) >= (filestate->timeLastRead + 1)) {
            int bytes_read = randBeetween(arguments.readMin, arguments.readMax);
            if (bytes_read > filestate->lastPositionInFile){
              bytes_read = filestate->lastPositionInFile;
            }
            if (bytes_read > 0) {
              char *val_read = calloc(bytes_read, sizeof(char));
              if (val_read == NULL) {
                printf("--- val_read is null!");
                exit(42);
              }

              fseek(filestate->file, 0, SEEK_SET); 
              fread(val_read, bytes_read, 1, filestate->file); 
      
              free(val_read);

              if (arguments.verbose) {
                printf("(%ld) [%2d | %2d ] Read %d Byte in File: %s\n", time(NULL), comm_rank, thread_me, bytes_read, filestate->filename);
              }
              filestate->iterationsRead++;
              filestate->timeLastRead = time(NULL);
            } else {
              if (arguments.verbose) {
                printf("(%ld) [%2d | %2d ] Read skip why 0 Bytes not possible to read: %s\n", time(NULL), comm_rank, thread_me, filestate->filename);
              }
            }
          }
        } else {
          printf("(%ld) [%2d | %2d ] Work with non init File at pos %d von %d\n", time(NULL), comm_rank, thread_me, randomFilePos, opendFiles);
          exit(-1);
        }
      }
      

      for(int i =0; i < fileCount; i++) {
          FileState *filestate = &fs[i];
          if (filestate->timeOpen == 0 || filestate->iterationsRead < arguments.iterations || filestate->iterationsWrite < arguments.iterations) {
            isDone = 0;
            break;
          }
          isDone = 1;
      }
    } while(!isDone);
    free(fs);
  } // END omp parallel



    MPI_CHECK(MPI_Finalize());
    return EXIT_SUCCESS;
}
