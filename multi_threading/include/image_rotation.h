#ifndef IMAGE_ROTATION_H_
#define IMAGE_ROTATION_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <stdint.h>
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define CHANNEL_NUM 1

#include "stb_image.h"
#include "stb_image_write.h"


/********************* [ Helpful Macro Definitions ] **********************/
#define BUFF_SIZE 1024 
#define LOG_FILE_NAME "request_log"               // Standardized log file name
#define INVALID -1                                // Reusable int for marking things as invalid or incorrect
#define MAX_THREADS 100                           // Maximum number of threads
#define MAX_QUEUE_LEN 200                         // Maximum queue length


/********************* [ Helpful Typedefs        ] ************************/
typedef struct request {
  char file_name[BUFF_SIZE];
  int rotation_angle;
} request_t;

typedef struct request_queue {
  request_t* queue;
  int front;
  int count;
} request_queue_t;

typedef struct processing_args {
  char *dir_path;
  int num_workers;
  int rotation_angle;
} processing_args_t;

/********************* [ Function Prototypes       ] **********************/
void *processing(void* args); 
void *worker(void* args); 
void log_pretty_print(FILE* to_write, int threadId, int requestNumber, char* file_name);

#endif  // IMAGE_ROTATION_H_
