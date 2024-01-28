#include "client.h"

#define PORT 1957  // Last 4 digits of Student ID
#define SERVER_IP "127.0.0.1"

#define TRUE 1
#define FALSE 0

int send_file(int socket, const char *filename) {
  // Open the file
  FILE* fp = fopen(filename, "rb");
  if (!fp) { 
    perror("fopen()");
    exit(EXIT_FAILURE);
  }

  // Set up the request packet for the server and send it
  struct stat info;
  if (fstat(fileno(fp), &info) == -1) {
    perror("fstat()");
    exit(EXIT_FAILURE);
  }
  packet_t* request = malloc(sizeof(packet_t));
  *request = (packet_t){0};
  request->size = info.st_size;
  char* serialized_data = serialize(request);
  
  if (send(socket, serialized_data, sizeof(request), 0) == -1) {
    perror("send()");
    exit(EXIT_FAILURE);
  }

  // Send the file data
  char* img_data = read_file(fp, info.st_size);
  send_data(socket, img_data, info.st_size);

  free(request);
  free(serialized_data);
  free(img_data);
  fclose(fp);

  return 0;
}

int receive_file(int socket, const char *filename) {
  // Open the file
  FILE* fp = fopen(filename, "wb");
  if (!fp) {
    perror("fopen()");
    exit(EXIT_FAILURE);
  }

  // Receive response packet
  char recvdata[BUFF_SIZE];
  packet_t* response;
  while (TRUE) {
    memset(recvdata, '\0', BUFF_SIZE);

    if (recv(socket, recvdata, sizeof(response), 0) == -1) {
      perror("recv()");
      exit(EXIT_FAILURE);
    }
    response = deserialize(recvdata);

    if (response->size)
      break;
    free(response);
  }

  // Receive the file data
  memset(recvdata, '\0', BUFF_SIZE);
  if (recv(socket, recvdata, BUFF_SIZE, 0) == -1) {
    perror("recv()");
    exit(EXIT_FAILURE);
  }

  char img_data[response->size];
  memset(img_data, '\0', response->size);
  memcpy(img_data, recvdata, sizeof(char)*response->size);

  // Write the data to the file
  fwrite(img_data, sizeof(char), response->size, fp);

  free(response);
  fclose(fp);

  return 0;
}

int main(int argc, char* argv[]) {
  if(argc != 4){
    fprintf(stderr, "Usage: ./client input_dir output_dir rotation_angle\n");
    exit(EXIT_FAILURE);
  }
  const char* inp_dir = argv[1];
  const char* out_dir = argv[2];
  const int rotate = atoi(argv[3]);

  // Set up socket
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  // Connect the socket
  struct sockaddr_in serv_addr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = inet_addr(SERVER_IP),
    .sin_port = htons(PORT)
  };

  if (connect(sock_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
    perror("connect()");
    exit(EXIT_FAILURE);
  }

  // Read the directory for all the images to rotate
  DIR* dir = opendir(inp_dir);
  if (!dir) {
    perror("opendir()");
    exit(EXIT_FAILURE);
  }
  struct dirent* dir_ent;
  request_t req_queue[MAX_QUEUE_LEN] = {0};

  unsigned int i = 0;
  while ((dir_ent = readdir(dir)) && (i < MAX_QUEUE_LEN)) {
    if (strcmp(dir_ent->d_name, ".") == 0)
      continue;
    if (strcmp(dir_ent->d_name, "..") == 0)
      continue;
    // *** if __APPLE__
    if (strcmp(dir_ent->d_name, ".DS_Store") == 0)
      continue;
    // ***

    if (dir_ent->d_type == DT_REG) {
      req_queue[i].rotation_angle = rotate;
      const int path_len = strlen(inp_dir) + 1 + strlen(dir_ent->d_name) + 1;
      char* file_path = malloc(sizeof(char)*path_len);
      strcpy(file_path, inp_dir);
      strcat(file_path, "/");
      strcat(file_path, dir_ent->d_name);
      req_queue[i].file_name = file_path;
      i++;
    }
  }

  const int count = i;
  for (i = 0; i < count; i++) {
    // Send the image data to the server
    packet_t* request = malloc(sizeof(packet_t));
    *request = (packet_t){IMG_OP_ROTATE, IMG_FLAG_ROTATE_180, 0};
    if (rotate == 270)
      request->flags = IMG_FLAG_ROTATE_270;
    char* serialized_data = serialize(request);

    if (send(sock_fd, serialized_data, sizeof(request), 0) == -1) {
      perror("send()");
      exit(EXIT_FAILURE);
    }
    send_file(sock_fd, req_queue[i].file_name);

    free(request);
    free(serialized_data);

    // Check that the request was acknowledged
    char recvdata[BUFF_SIZE];
    packet_t* response;

    while (TRUE) {
      memset(recvdata, '\0', BUFF_SIZE);

      if (recv(sock_fd, recvdata, sizeof(response), 0) == -1) {
        perror("recv()");
        exit(EXIT_FAILURE);
      }
      response = deserialize(recvdata);

      if (
        response->operation == IMG_OP_ACK ||
        response->operation == IMG_OP_NAK
      ) break;

      free(response);
    }
  
    // Receive the processed image and save it in the output dir
    if (response->operation == IMG_OP_ACK) {
      const char* filename = get_filename_from_path(req_queue[i].file_name);
      char file_path[strlen(out_dir) + 1 + strlen(filename) + 1];
      strcpy(file_path, out_dir);
      strcat(file_path, "/");
      strcat(file_path, filename);
      receive_file(sock_fd, file_path);
    }
    free(response);
  }

  // Terminate the connection once all images have been processed
  packet_t* request = malloc(sizeof(packet_t));
  *request = (packet_t){0};
  request->operation = IMG_OP_EXIT;
  char* serialized_data = serialize(request);

  if (send(sock_fd, serialized_data, sizeof(request), 0) == -1) {
    perror("send()");
    exit(EXIT_FAILURE);
  }
  free(request);
  free(serialized_data);

  // Release any resources
  for (i = 0; i < count; i++)
    free(req_queue[i].file_name);

  close(sock_fd);
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

char* read_file(FILE* fd, const int size) {
  char* buff = malloc(sizeof(char)*size);
  memset(buff, '\0', size);
  fread(buff, sizeof(char), size, fd);
  return buff;
}

void send_data(int socket, const char* file_data, const int size) {
  int nbytes = 0;
  while (nbytes < size) {
    int ret = send(socket, file_data, sizeof(char)*size, 0);
    if (ret == -1) {
      perror("send()");
      exit(EXIT_FAILURE);
    }
    nbytes += ret;
  }
}
