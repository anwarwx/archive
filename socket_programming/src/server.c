#include "server.h"

#define PORT 1957  // Last 4 digits of Student ID
#define MAX_CLIENTS 5

#define TRUE 1
#define FALSE 0
#define VALID 0
#define INVALID 1

void *clientHandler(void* socket) {
  sargs_t sargs = *(sargs_t*) socket;
  int id = sargs.id;
  int sock_fd = sargs.fd;

  // Receive packets from the client
  char recvdata[BUFF_SIZE];
  packet_t* request;
  while (TRUE) {
    memset(recvdata, '\0', BUFF_SIZE);

    if (recv(sock_fd, recvdata, sizeof(request), 0) == -1) {
      perror("recv()");
      exit(EXIT_FAILURE);
    }
    request = deserialize(recvdata);

    // Determine the packet operatation and flags
    if (request->operation == IMG_OP_EXIT) {
      free(request);
      close(sock_fd);
      printf("-- TERMINATING THREAD %d --\n", id);
      pthread_exit(NULL);
    }

    if (request->operation == IMG_OP_ROTATE) {
      int flags = request->flags;

      while (TRUE) {
        memset(recvdata, '\0', BUFF_SIZE);

        if (recv(sock_fd, recvdata, sizeof(request), 0) == -1) {
          perror("recv()");
          exit(EXIT_FAILURE);
        }
        free(request);
        request = deserialize(recvdata);

        if (request->size) {
          // Receive the image data using the size
          memset(recvdata, '\0', BUFF_SIZE);

          if (recv(sock_fd, recvdata, BUFF_SIZE, 0) == -1) {
            perror("recv()");
            exit(EXIT_FAILURE);
          }

          packet_t* response = malloc(sizeof(packet_t));
          *response = (packet_t){0};
          response->operation = IMG_OP_ACK;

          char img_data[request->size];
          memset(img_data, '\0', sizeof(char)*request->size);
          memcpy(img_data, recvdata, sizeof(char)*request->size);

          // Process the image data based on the set of flags
          char temporary[8];
          sprintf(temporary, "%d.png", id);

          if (write_file(img_data, temporary, request->size))
            response->operation = IMG_OP_NAK;
          else
            rotate(temporary, flags);

          // Acknowledge the request and return the processed image data
          char* serialized_data = serialize(response);

          if (send(sock_fd, serialized_data, sizeof(response), 0) == -1) {
            perror("send()");
            exit(EXIT_FAILURE);
          }
          free(serialized_data);

          int len;
          char* png = read_file(temporary, &len);

          response->size = len;
          serialized_data = serialize(response);
          if (send(sock_fd, serialized_data, sizeof(response), 0) == -1) {
            perror("send()");
            exit(EXIT_FAILURE);
          }
          send_data(sock_fd, png, len);

          free(serialized_data);
          free(request);
          free(response);

          break;
        }
      }
    }
  }
}

int main(int argc, char* argv[]) {
  // Creating socket file descriptor
  int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd == -1) {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in serv_addr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = htonl(INADDR_ANY),
    .sin_port = htons(PORT)
  };

  // Bind the socket to the port
  if (bind(listen_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
    perror("bind()");
    exit(EXIT_FAILURE);
  }

  // Listen on the socket
  if (listen(listen_fd, MAX_CLIENTS) == -1) {
    perror("listen()");
    exit(EXIT_FAILURE);
  }

  // Accept connections and create the client handling threads
  struct sockaddr_in cli_addr;
  pthread_t thds[MAX_THREADS];
  int conn_fds[MAX_THREADS];
  unsigned int i = 0;

  while (TRUE) {
    conn_fds[i] = accept(listen_fd, (struct sockaddr*) &cli_addr, &(socklen_t){sizeof(cli_addr)});
    if (conn_fds[i] == -1)
      perror("accept()");

    sargs_t sargs = {i, conn_fds[i]};
    if (pthread_create(&thds[i], NULL, (void*) clientHandler, &sargs))
      perror("pthread_create()");

    if (pthread_detach(thds[i]))
      perror("pthread_detach()");
    i++;
  }

  // Release any resources
  close(listen_fd);
  exit(EXIT_SUCCESS);
}



/********************* [ Helpful Functions        ] ************************/
char* serialize(packet_t* packet) {
  char* data = malloc(sizeof(char)*sizeof(packet_t));
  memset(data, '\0', sizeof(packet_t));
  memcpy(data, packet, sizeof(packet_t));
  return data;
}

packet_t* deserialize(char* data) {
  packet_t* packet = malloc(sizeof(packet_t));
  memset(packet, 0, sizeof(packet_t));
  memcpy(packet, data, sizeof(packet_t));
  return packet;
}

int write_file(const char* file_data, const char* filename, const int size) {
  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    perror("fopen()");
    return INVALID;
  }
  fwrite(file_data, sizeof(char), size, fp);
  fclose(fp);
  return VALID;
}

void rotate(const char* filename, const int flag) {
  int w, h, bpp;
  uint8_t* img_result =
    stbi_load(filename, &w, &h, &bpp, CHANNEL_NUM);
  uint8_t** result_mat = malloc(sizeof(uint8_t*)*w);
  uint8_t** img_mat = malloc(sizeof(uint8_t*)*w);
  unsigned int i = 0;
  for (; i < w; i++) {
    result_mat[i] = malloc(sizeof(uint8_t)*h);
    img_mat[i] = malloc(sizeof(uint8_t)*h);
  }
  linear_to_image(img_result, img_mat, w, h);
  if (flag == IMG_FLAG_ROTATE_180)
    flip_left_to_right(img_mat, result_mat, w, h);
  else if (flag == IMG_FLAG_ROTATE_270)
    flip_upside_down(img_mat, result_mat, w, h);
  uint8_t* img_array = malloc(sizeof(uint8_t)*w*h);
  flatten_mat(result_mat, img_array, w, h);
  stbi_write_png(filename, w, h, CHANNEL_NUM, img_array, w*CHANNEL_NUM);
  stbi_image_free(img_result);
  for (i = 0; i < w; i++) {
    free(result_mat[i]);
    free(img_mat[i]);
  }
  free(result_mat);
  free(img_mat);
  free(img_array);
}

char* read_file(const char* filename, int* size) {
  FILE* fp = fopen(filename, "rb");
  if(!fp) {
    perror("fopen()");
    return NULL;
  }
  struct stat info;
  if (fstat(fileno(fp), &info) == -1) {
    perror("fstat()");
    return NULL;
  }
  *size = info.st_size;
  char* buff = malloc(sizeof(char)**size);
  memset(buff, '\0', *size);
  fread(buff, sizeof(char), *size, fp);
  fclose(fp);
  return buff;
}

int send_data(int socket, const char* file_data, const int size) {
  int nbytes = 0;
  while (nbytes < size) {
    int ret = send(socket, file_data, sizeof(char)*size, 0);
    if (ret == -1) {
      perror("send()");
      return INVALID;
    }
    nbytes += ret;
  }
  return VALID;
}
