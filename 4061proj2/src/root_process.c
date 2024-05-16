#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/utils.h"

#define WRITE (O_WRONLY | O_CREAT | O_TRUNC)
#define PERM (S_IRUSR | S_IWUSR)
#define BUFSIZE 1024
char *output_file_folder = "output/final_submission/";

void redirection(char **dup_list, int size, char* root_dir){
    //(overview): redirect standard output to an output file in output_file_folder("output/final_submission/")
    //(step1): determine the filename based on root_dir. e.g. if root_dir is "./root_directories/root1", the output file's name should be "root1.txt"
    char output_filename[BUFSIZE];

    // Extracting the base name from the root_dir path
    char* base = strrchr(root_dir, '/'); //gets pointer to the last location of '/'
    if (base) {
        base++;  // Move past the '/'
    } else {
        base = root_dir;  // If no '/', the entire path is the basename
    }

    snprintf(output_filename, BUFSIZE, "%s%s.txt", output_file_folder, base);

    //(step2): redirect standard output to output file (output/final_submission/root*.txt)
    int fd = open(output_filename, WRITE, PERM);
    dup2(fd, STDOUT_FILENO);
    close(fd);

    //(step3): read the content each symbolic link in dup_list, write the path as well as the content of symbolic link to output file(as shown in expected)
    for (int i = 0; i < size; i++) {
        char buffer[BUFSIZE];
        ssize_t bytes_read = readlink(dup_list[i], buffer, BUFSIZE);
        if (bytes_read != -1) {
            buffer[bytes_read] = '\0';  // Null-terminate the read content
            printf("[<path of symbolic link> --> <path of retained file>] : [%s --> %s]\n", dup_list[i], buffer);
        }
    }
}


// helper
void create_symlinks(char **dup_list, char **retain_list, int size) {
    //TODO(): create symbolic link at the location of deleted duplicate file
    //TODO(): dup_list[i] will be the symbolic link for retain_list[i]

    for (int i = 0; i < size; i++) {
        if (symlink(retain_list[i], dup_list[i]) != 0) {
            perror("Error with creating symbolic link between retain_list and dup_list");
        }
    }
}

// helper
void delete_duplicate_files(char **dup_list, int size) {
    //TODO(): delete duplicate files, each element in dup_list is the path of the duplicate file
    for (int i = 0; i < size; i++) {
        if (remove(dup_list[i]) != 0) {
            perror("Error with deleting duplicate files");
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
    char all_filepath_hashvalue[4098]; //buffer for gathering all data transferred from child process
    memset(all_filepath_hashvalue, 0, sizeof(all_filepath_hashvalue));// clean the buffer

    //TODO(step1): construct pipe
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Failed to create pipe");
    }

    //TODO(step2): fork() child process & read data from pipe to all_filepath_hashvalue
    pid_t pid = fork();
    if (pid == -1) {
        perror("Failed to fork");
    }
    
    if (pid == 0) {  // Child process
        //printf("In child process, about to run nonleaf_process...\n");
        char fd_w_str[10];
        sprintf(fd_w_str, "%d", fd[1]); //print fd[1] to fd write string

        close(fd[0]); //close read end of pipe
        dup2(fd[1], STDOUT_FILENO); //redirect stdout to the write end
        //close(fd[1]); //close fd[1] since stdout points to it now
        
        execl("./nonleaf_process", "nonleaf_process", root_directory, fd_w_str, (char *) NULL);
        perror("execl failed");
    } else { // parent process
    	wait(NULL); // See below
        //printf("In parent process, after child completion...\n");
        close(fd[1]);
        // TODO TODO Should the parent process wait for the child process to complete everything with the children? TODO TODO
        ssize_t bytesRead = read(fd[0], all_filepath_hashvalue, sizeof(all_filepath_hashvalue) - 1);  // Adjusted the size by -1 for the null terminator
        if (bytesRead == -1) {
            perror("Error reading from pipe");
            exit(EXIT_FAILURE);
        }
        all_filepath_hashvalue[bytesRead] = '\0';  // Null-terminate the string after reading
        //printf("Read from child: %s\n", all_filepath_hashvalue);  // Check if data is correctly filled
        close(fd[0]);
    }
    
    //printf("Final state of all_filepath_hashvalue: %s\n", all_filepath_hashvalue);  // Final State


    //TODO(step3): malloc dup_list and retain list & use parse_hash() in utils.c to parse all_filepath_hashvalue
    // dup_list: list of paths of duplicate files. We need to delete the files and create symbolic links at the location
    // retain_list: list of paths of unique files. We will create symbolic links for those files
    char** dup_list = malloc(10 * sizeof(char*));
    char** retain_list = malloc(10 * sizeof(char*));
    if (!dup_list || !retain_list) {
        //fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        dup_list[i] = NULL;
        retain_list[i] = NULL;
    }

    int size = parse_hash(all_filepath_hashvalue, dup_list, retain_list);

    //TODO(step4): implement the functions
    delete_duplicate_files(dup_list,size);
    create_symlinks(dup_list, retain_list, size);
    redirection(dup_list, size, argv[1]);

    //TODO(step5): free any arrays that are allocated using malloc!!
    for (int i = 0; i < size; i++) {
        free(dup_list[i]);
        free(retain_list[i]);
    }
    free(dup_list);
    free(retain_list);
}
