#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/hash.h"
#include "../include/utils.h"

#define HASH_SIZE SHA256_BLOCK_SIZE * 2 + 1

char *output_file_folder = "output/inter_submission/";

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: Inter Submission --> ./leaf_process <file_path> 0\n");
        printf("Usage: Final Submission --> ./leaf_process <file_path> <pipe_write_end>\n");
        return -1;
    }
    //TODO(): get <file_path> <pipe_write_end> from argv[]
    char *PATH = argv[1];
    const int FD_1 = atoi(argv[2]);

    //TODO(): create the hash of given file
    char result_hash[HASH_SIZE];
    memset(result_hash, '\0', sizeof(result_hash));
    hash_data_block(result_hash, PATH);

    //TODO(): construct string write to pipe. The format is "<file_path>|<hash_value>|"
    char str_buf[strlen(PATH) + 1 + HASH_SIZE + 1 + 1];
    strcpy(str_buf, PATH);
    strcat(str_buf, "|");
    strcat(str_buf, result_hash);
    strcat(str_buf, "|");

    if (FD_1 == 0) {
        char *filename = extract_filename(PATH);
        char *root_dir = extract_root_directory(PATH);
        char file_loc[strlen(output_file_folder)+strlen(root_dir)+1];
        strcpy(file_loc, output_file_folder);
        strcat(file_loc, root_dir);
        if (chdir(file_loc)) return -1;
        FILE* fd = fopen(filename, "w");
        if (!fd) return -1;
        fwrite(str_buf, sizeof(char), strlen(str_buf), fd);
        fclose(fd);
        free(root_dir);
        exit(0);
    }

    if (write(FD_1, str_buf, strlen(str_buf)) == -1) exit(EXIT_FAILURE);
    close(FD_1);
    exit(0);
}
