#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/utils.h"
#include <sys/wait.h>

#define DATA_SIZE 4098

#define WRITE (O_WRONLY | O_CREAT | O_TRUNC)
#define PERM (S_IRUSR | S_IWUSR)
char *output_file_folder = "output/final_submission/";

void redirection(char **dup_list, int size, char* root_dir){
    // TODO(overview): redirect standard output to an output file in output_file_folder("output/final_submission/")
    // TODO(step1): determine the filename based on root_dir. e.g. if root_dir is "./root_directories/root1", the output file's name should be "root1.txt"
    char *file_dir = extract_filename(root_dir);
    char *file_typ = ".txt";

    char filename[strlen(file_dir) + strlen(file_typ) + 1];
    strcpy(filename, file_dir);
    strcat(filename, file_typ);

    //TODO(step2): redirect standard output to output file (output/final_submission/root*.txt)
    if (chdir(output_file_folder)) { perror("chdir()"); exit(EXIT_FAILURE); }
    int fd = open(filename, WRITE, PERM);
    if (!fd) { perror("open()"); exit(EXIT_FAILURE); }
    for (int i = 0; i < 2; i++) { if (chdir("..")) { perror("chdir()"); exit(EXIT_FAILURE); } }
    const int FD_OUT = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);
    close(fd);

    //TODO(step3): read the content each symbolic link in dup_list, write the path as well as the content of symbolic link to output file(as shown in expected)
    for (unsigned int i = 0; i < size; i++) {
        char slink_path[PATH_MAX];
        memset(slink_path, '\0', sizeof(char)*PATH_MAX);
        readlink(dup_list[i], slink_path, sizeof(char)*PATH_MAX);

        printf("[<path of symbolic link> --> <path of retained file>] : ");
        printf("[%s --> %s]\n", dup_list[i], slink_path);
        fflush(stdout);
    }

    dup2(FD_OUT, STDOUT_FILENO);
    close(FD_OUT);
    return;
}

void create_symlinks(char **dup_list, char **retain_list, int size) {
    //TODO(): create symbolic link at the location of deleted duplicate file
    //TODO(): dup_list[i] will be the symbolic link for retain_list[i]
    for (unsigned int i = 0; i < size; i++) {
        if (symlink(retain_list[i], dup_list[i])) {
            perror("symlink()");
            exit(EXIT_FAILURE);
        }
    }

}

void delete_duplicate_files(char **dup_list, int size) {
    //TODO(): delete duplicate files, each element in dup_list is the path of the duplicate file
    for (unsigned int i = 0; i < size; i++) {
        if (remove(dup_list[i])) {
            perror("remove()");
            exit(EXIT_FAILURE);
        }
    }
}

// ./root_directories <directory>
int main(int argc, char* argv[]) {
    if (argc != 2) {
        // dir is the root_directories directory to start with
        // e.g. ./root_directories/root1
        printf("Usage: ./root <dir> \n");
        return 1;
    }

    //TODO(overview): fork the first non_leaf process associated with root directory("./root_directories/root*")

    char* root_directory = argv[1];
    char all_filepath_hashvalue[DATA_SIZE]; //buffer for gathering all data transferred from child process
    memset(all_filepath_hashvalue, '\0', sizeof(char)*DATA_SIZE);// clean the buffer

    //TODO(step1): construct pipe
    int fds[2];
    if (pipe(fds)) exit(EXIT_FAILURE);

    //TODO(step2): fork() child process & read data from pipe to all_filepath_hashvalue
    pid_t pid = fork();
    if (pid == -1) exit(EXIT_FAILURE);
    if (pid == 0) {
        char fd[1024]; memset(fd, 0, sizeof(fd));
        sprintf(fd, "%d", fds[1]);

        char *args[4] = {"./nonleaf_process", root_directory, fd, NULL};
        execv(args[0], args);
    }
    if (pid) {
        waitpid(pid, NULL, 0);
        close(fds[1]);
        read(fds[0], all_filepath_hashvalue, sizeof(char)*DATA_SIZE);
        close(fds[0]);
    }

    //TODO(step3): malloc dup_list and retain list & use parse_hash() in utils.c to parse all_filepath_hashvalue
    // dup_list: list of paths of duplicate files. We need to delete the files and create symbolic links at the location
    // retain_list: list of paths of unique files. We will create symbolic links for those files
    char *dup_list[PATH_MAX];
    char *retain_list[PATH_MAX];
    int size = parse_hash(all_filepath_hashvalue, dup_list, retain_list);

    //TODO(step4): implement the functions
    delete_duplicate_files(dup_list, size);
    create_symlinks(dup_list, retain_list, size);
    redirection(dup_list, size, argv[1]);

    //TODO(step5): free any arrays that are allocated using malloc!!
    return 0;
}
