#include "image_rotation.h"

#define TRUE 1
#define FALSE 0

FILE* fd = NULL;
char* out_dir = NULL;

pthread_t* workers = NULL;
int num_workers = 0;

pthread_mutex_t
  queu_lock = PTHREAD_MUTEX_INITIALIZER,
  file_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t
  proc_cond = PTHREAD_COND_INITIALIZER,
  work_cond = PTHREAD_COND_INITIALIZER;

request_queue_t* shared_queue = NULL;
int* thread_reqs = NULL;
int* thread_exit = NULL;

/*
    The Function takes:
    to_write: A file pointer of where to write the logs. 
    requestNumber: the request number that the thread just finished.
    file_name: the name of the file that just got processed. 

    The function output: 
    it should output the threadId, requestNumber, file_name into the logfile and stdout.
*/
void log_pretty_print(FILE* to_write, int threadId, int requestNumber, char* file_name) {
  fprintf(to_write, "[%d][%d][%s]\n", threadId, requestNumber, file_name);
  printf("[%d][%d][%s]\n", threadId, requestNumber, file_name);
}

/*

    1: The processing function takes a void* argument called args. It is expected to be a pointer to a structure processing_args_t 
    that contains information necessary for processing.

    2: The processing thread need to traverse a given dictionary and add its files into the shared queue while maintaining synchronization using lock and unlock. 

    3: The processing thread should pthread_cond_signal/broadcast once it finish the traversing to wake the worker up from their wait.

    4: The processing thread will block(pthread_cond_wait) for a condition variable until the workers are done with the processing of the requests and the queue is empty.

    5: The processing thread will cross check if the condition from step 4 is met and it will signal to the worker to exit and it will exit.

*/
void *processing(void *args) {
  processing_args_t p_args = *(processing_args_t*) args;

  DIR* dir = opendir(p_args.dir_path);
  if (!dir) { perror("opendir"); exit(EXIT_FAILURE); }
  struct dirent* dir_ent;

  unsigned int i = 0;
  for (; (dir_ent = readdir(dir)) && (i < MAX_QUEUE_LEN); i++) {
    if (strcmp(dir_ent->d_name, ".") == 0) dir_ent = readdir(dir);
    if (strcmp(dir_ent->d_name, "..") == 0) dir_ent = readdir(dir);
    // *** if __APPLE__
    if (strcmp(dir_ent->d_name, ".DS_Store") == 0) dir_ent = readdir(dir);
    // ***
    if (dir_ent->d_type == DT_REG || dir_ent->d_type == DT_UNKNOWN) {
      request_t new_req = {"\0", p_args.rotation_angle};
      strcpy(new_req.file_name, p_args.dir_path);
      strcat(new_req.file_name, "/");
      strcat(new_req.file_name, dir_ent->d_name);

      pthread_mutex_lock(&queu_lock);
      shared_queue->queue[i] = new_req;
      shared_queue->count++;
      pthread_cond_broadcast(&work_cond);
      pthread_mutex_unlock(&queu_lock);
    }
  }
  int num_reqs = i;
  pthread_mutex_lock(&queu_lock);
  while (TRUE) {
    while (shared_queue->count) {
      pthread_cond_wait(&proc_cond, &queu_lock);
    }
    int total = 0;
    for (i = 0; i < num_workers; i++) total += thread_reqs[i];

    if (total == num_reqs) {
      for (i = 0; i < num_workers; i++) thread_exit[i] = TRUE;
      pthread_cond_broadcast(&work_cond);
      pthread_mutex_unlock(&queu_lock);
      break;
    }
  }

  if (closedir(dir)) { perror("closedir"); exit(EXIT_FAILURE); }
  pthread_exit(NULL);
}

/*
    1: The worker threads takes an int ID as a parameter

    2: The Worker thread will block(pthread_cond_wait) for a condition variable that there is a requests in the queue. 

    3: The Worker threads will also block(pthread_cond_wait) once the queue is empty and wait for a signal to either exit or do work.

    4: The Worker thread will processes request from the queue while maintaining synchronization using lock and unlock. 

    5: The worker thread will write the data back to the given output dir as passed in main. 

    6: The Worker thread will log the request from the queue while maintaining synchronization using lock and unlock.  

    8: Hint the worker thread should be in a While(1) loop since a worker thread can process multiple requests and It will have two while loops in total
        that is just a recommendation feel free to implement it your way :) 
    9: You may need different lock depending on the job.  

*/
void* worker(void *args) {
  int id = *(int*) args;

  while (TRUE) {
    pthread_mutex_lock(&queu_lock);
    while (shared_queue->count == 0) {
      if (thread_exit[id]) {
        pthread_mutex_unlock(&queu_lock);
        pthread_exit(NULL);
      }
      pthread_cond_wait(&work_cond, &queu_lock);
    }

    request_t dequeue = shared_queue->queue[shared_queue->front];
    shared_queue->front++;
    shared_queue->front%=MAX_QUEUE_LEN;
    shared_queue->count--;

    pthread_cond_signal(&proc_cond);
    pthread_mutex_unlock(&queu_lock);

    // vectorize image
    int width, height, bpp;
    uint8_t* image_result = stbi_load(dequeue.file_name, &width, &height, &bpp, CHANNEL_NUM);

    uint8_t** result_matrix = (uint8_t**) malloc(sizeof(uint8_t*)*width);
    uint8_t** img_matrix = (uint8_t**) malloc(sizeof(uint8_t*)*width);
    for (unsigned int i = 0; i < width; i++) {
      result_matrix[i] = (uint8_t*) malloc(sizeof(uint8_t)*height);
      img_matrix[i] = (uint8_t*) malloc(sizeof(uint8_t)*height);
    }

    // convert image vector into a matrix
    linear_to_image(image_result, img_matrix, width, height);

    if (dequeue.rotation_angle == 180) flip_left_to_right(img_matrix, result_matrix, width, height);
    else if (dequeue.rotation_angle == 270) flip_upside_down(img_matrix, result_matrix, width, height);
      
    uint8_t* img_array = malloc(sizeof(uint8_t) * width * height);
      
    // flatten new image matrix back into vector
    flatten_mat(result_matrix, img_array, width, height);

    const char* file_name = get_filename_from_path(dequeue.file_name);
    char new_path[strlen(out_dir) + 1 + strlen(file_name) + 1];
    strcpy(new_path, out_dir);
    strcat(new_path, "/");
    strcat(new_path, file_name);
      
    // write new vector image to a new png
    stbi_write_png(new_path, width, height, CHANNEL_NUM, img_array, width*CHANNEL_NUM);
    thread_reqs[id]++;

    pthread_mutex_lock(&file_lock);
    log_pretty_print(fd, id, thread_reqs[id], dequeue.file_name);
    pthread_mutex_unlock(&file_lock);

    stbi_image_free(image_result);
    for (unsigned int i = 0; i < width; i++) {
      free(result_matrix[i]);
      free(img_matrix[i]);
    }
    free(result_matrix);
    free(img_matrix);
    free(img_array);
  }
}

/*
  Main:
    Get the data you need from the command line argument 
    Open the logfile
    Create the threads needed
    Join on the created threads
    Clean any data if needed. 
*/
int main(int argc, char* argv[]) {
  if (argc != 5) {
    fprintf(stderr, "Usage: <./prog_name> <img_dir_path> <output_dir_path> <num_worker_threads> <rotation_angle>\n");
    exit(EXIT_FAILURE);
  }

  char* img_dir = argv[1];
  out_dir = argv[2];
  num_workers = atoi(argv[3]);

  if (num_workers > MAX_THREADS) {
    fprintf(stderr, "# of threads < 100\n");
    exit(EXIT_FAILURE);
  }

  const int rotate = atoi(argv[4]);

  fd = fopen(LOG_FILE_NAME, "w");
  if (!fd) exit(EXIT_FAILURE);

  pthread_t p_thd;
  workers = malloc(sizeof(pthread_t)*num_workers);

  thread_reqs = malloc(sizeof(int)*num_workers);
  thread_exit = malloc(sizeof(int)*num_workers);

  memset(thread_reqs, 0, sizeof(int)*num_workers);
  memset(thread_exit, FALSE, sizeof(int)*num_workers);

  shared_queue = malloc(sizeof(request_queue_t));
  shared_queue->queue = malloc(sizeof(request_t)*MAX_QUEUE_LEN);
  shared_queue->front = 0;
  shared_queue->count = 0;

  processing_args_t p_args = {img_dir, num_workers, rotate};
  if (pthread_create(&p_thd, NULL, (void*)processing, (void*)&p_args)) {
    fprintf(stderr, "pthread_create() : processing()\n");
    exit(EXIT_FAILURE);
  }

  unsigned int i = 0;

  int ids[num_workers];
  for (int id = 0; i < num_workers; i++, id++) {
    ids[i] = id;
    if (pthread_create(&workers[i], NULL, (void*)worker, (void*)&ids[i])) {
      fprintf(stderr, "pthread_create() : worker()\n");
      exit(EXIT_FAILURE);
    }
  }

  pthread_join(p_thd, NULL);
  for (i = 0; i < num_workers; i++) pthread_join(workers[i], NULL);

  if (fclose(fd)) exit(EXIT_FAILURE);

  pthread_mutex_destroy(&queu_lock);
  pthread_mutex_destroy(&file_lock);
  pthread_cond_destroy(&proc_cond);
  pthread_cond_destroy(&work_cond);

  free(workers);
  free(shared_queue->queue);
  free(shared_queue);
  free(thread_reqs);
  free(thread_exit);
  
  workers = NULL;
  shared_queue = NULL;
  thread_reqs = NULL;
  thread_exit = NULL;

  exit(EXIT_SUCCESS);
}
