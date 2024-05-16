#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define BUFSIZE 1024

int main(int argc, char* argv[]) {
    // Ensure that the correct number of arguments are provided
    if (argc != 3) {
        printf("Usage: ./nonleaf_process <directory_path> <pipe_write_end> \n");
        return 1;
    }

    char *directory_path = argv[1];
    int pipe_write_end = atoi(argv[2]);

    // Allocate buffer for reading from child processes
    char* buffer = (char*) malloc(BUFSIZE * sizeof(char));
    if (!buffer) {
        perror("Failed to allocate memory for buffer");
        exit(1);
    }
    memset(buffer, 0, BUFSIZE);

    // Open the directory for reading
    DIR *dir = opendir(directory_path);
    if (dir == NULL) {
        perror("Failed to open directory");
        free(buffer);
        exit(1);
    }

    // Count the number of entries in the directory for dynamic allocation
    int num_entries = 0;
    while (readdir(dir) != NULL) num_entries++;
    rewinddir(dir);  // Reset the directory stream position

    // Allocate an array to store child process pipe descriptors
    int *child_pipes = (int *)malloc(num_entries * sizeof(int));
    if (!child_pipes) {
        perror("Failed to allocate memory for child_pipes");
        free(buffer);
        closedir(dir);
        exit(1);
    }

    struct dirent *entry;
    int num_children = 0;

    //fprintf(stderr, "Starting nonleaf for directory: %s\n", directory_path);

    // Iterate through each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip the '.' and '..' entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        // Create a pipe for communication with the child process
        int fd[2];
        if (pipe(fd) == -1) {
            perror("Failed to create pipe");
            exit(1);
        }

        pid_t pid = fork();  // Create a child process
        if (pid == -1) {
            perror("Failed to fork");
            exit(1);
        }

        if (pid == 0) {  // Child process code
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);  // Redirect stdout to the write end of the pipe
            //close(fd[1]);
            
            char file_path[BUFSIZE];
            snprintf(file_path, BUFSIZE-1, "%s/%s", directory_path, entry->d_name);
            
            char fd_w_str[10];
            snprintf(fd_w_str, sizeof(fd_w_str)-1, "%d", fd[1]);

            // If the entry is a directory, call nonleaf_process recursively; if it's a file, call leaf_process
            if (entry->d_type == DT_DIR) {
                //fprintf(stderr, "Executing nonleaf_process for directory: %s\n", file_path);
                execl("./nonleaf_process", "nonleaf_process", file_path, fd_w_str, (char*)NULL);
            } else if (entry->d_type == DT_REG) {
                //fprintf(stderr, "Executing leaf_process for file: %s\n", file_path);
                execl("./leaf_process", "leaf_process", file_path, fd_w_str, (char*)NULL);
            }
            
            perror("execl failed");
            exit(1);
        } else {  // Parent process code
            close(fd[1]);
            child_pipes[num_children++] = fd[0];  // Store the read end of the pipe
        }
    }
    
    //make sure all children finish before reading
    for (int i = 0; i < num_children; i++) { 
        wait(NULL);
    }


    // Read data from each child process and write it to the parent process's pipe
    for (int i = 0; i < num_children; i++) {
        ssize_t bytes_read;
        while ((bytes_read = read(child_pipes[i], buffer, BUFSIZE-1)) > 0) {
            buffer[bytes_read] = '\0';
            ssize_t bytes_written = write(pipe_write_end, buffer, bytes_read);
            if (bytes_written < 0) {
                perror("Failed to write to pipe");
            }
        }
        close(child_pipes[i]);
    }

    close(pipe_write_end); // Close the write end
    //fprintf(stderr, "Nonleaf process for directory: %s completed.\n", directory_path);

    // Clean up
    closedir(dir);
    free(buffer);
    free(child_pipes);
    return 0;
}
