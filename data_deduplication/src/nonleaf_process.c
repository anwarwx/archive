#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define DATA_SIZE 4098

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./nonleaf_process <directory_path> <pipe_write_end> \n");
        return 1;
    }

    //TODO(overview): fork the child processes(non-leaf process or leaf process) each associated with items under <directory_path>
    //TODO(step1): get <file_path> <pipe_write_end> from argv[]
    const char *DIR_PATH = argv[1];
    const int FD_1 = atoi(argv[2]);

    //TODO(step2): malloc buffer for gathering all data transferred from child process as in root_process.c
    char all_filepath_hashvalue[DATA_SIZE];

    //TODO(step3): open directory
    DIR *dir = opendir(DIR_PATH);
    if (!dir) { perror("opendir"); return 1; }
    
    struct dirent *dir_ent;

    //TODO(step4): traverse directory and fork child process
    // Hints: Maintain an array to keep track of each read end pipe of child process
    pid_t pid;
    int read_pipes[1024];
    for (unsigned int i = 0; (dir_ent = readdir(dir)); i++) {
        if (strcmp(dir_ent->d_name, ".") == 0) dir_ent = readdir(dir);
        if (strcmp(dir_ent->d_name, "..") == 0) dir_ent = readdir(dir);

        char abs_path[strlen(DIR_PATH) + 1 + strlen(dir_ent->d_name) + 1];
        strcpy(abs_path, DIR_PATH);
        strcat(abs_path, "/");
        strcat(abs_path, dir_ent->d_name);

        int fds[2];
        if (pipe(fds)) exit(EXIT_FAILURE);

        read_pipes[i] = fds[0];

        char fd[5] = "";
        sprintf(fd, "%d", fds[1]);

        pid = fork();
        if (pid == -1) exit(EXIT_FAILURE);
        if (pid == 0) {
            if (dir_ent->d_type == DT_DIR) {
                close(fds[0]);
                char* args[4] = {"./nonleaf_process", abs_path, fd, NULL};
                execv(args[0], args);
            }
            else if (dir_ent->d_type == DT_REG) {
                close(fds[0]);
                char* args[4] = {"./leaf_process", abs_path, fd, NULL};
                execv(args[0], args);
            }
        }
        if (pid) { close(fds[1]); }
    }

    //TODO(step5): read from pipe constructed for child process and write to pipe constructed for parent process
    if (pid) {
        while (wait(NULL) > 0);
        for (unsigned int i = 0; read_pipes[i] != '\0'; i++) {
            memset(all_filepath_hashvalue, '\0', sizeof(char)*DATA_SIZE);
            read(read_pipes[i], all_filepath_hashvalue, sizeof(char)*DATA_SIZE);
            close(read_pipes[i]);
            write(FD_1, all_filepath_hashvalue, strlen(all_filepath_hashvalue));
        }
    }
    return 0;
}
