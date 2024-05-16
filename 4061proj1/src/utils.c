#include "utils.h"
#include <string.h>
#include <math.h>

// Create N files and distribute the data from the input file evenly among them
// See section 3.1 of the project writeup for important implementation details
void partition_file_data(char *input_file, int n, char *blocks_folder)
{

    // open the input file
    FILE *fd = fopen(input_file, "r");

    // code derived from the following posts
    // https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
    // https://stackoverflow.com/questions/7775027/how-to-create-file-of-x-size

    // seek to end of file and getting the size
    fseek(fd, 0L, SEEK_END);
    int size = ftell(fd);

    // not without going back for seperate I/O
    rewind(fd);

    // debug message
    printf("Size: %d\n", size);

    // standard size of files
    int new_size = floor((size) / n);
    // different size for last file
    int last_new_size = floor((size) / n) + (size) % n;

    // for loop to cycle through string except last one
    for (int i = 0; i < n; i++)
    {
        // Gets string to path
        char write_file_path[PATH_MAX];
        sprintf(write_file_path, "%s/%d.txt", blocks_folder, i);

        // Gets size of file with a check if its the last file
        size_t current_size;
        if (i != n - 1)
        {
            current_size = new_size;
        }
        else
        {
            current_size = last_new_size;
        }

        // Creates dynamically allocated buffer with current size
        char *buffer = malloc(current_size * sizeof(char));
        size_t bytesRead = fread(buffer, 1, current_size, fd);

        // open the file itself for writing
        FILE *fp = fopen(write_file_path, "w");
        fwrite(buffer, 1, bytesRead, fp);

        // debug statement
        printf("File size of %s: %zu\n", write_file_path, current_size);

        // close the file
        fclose(fp);
        free(buffer);
    }
    fclose(fd);
}

// ##### DO NOT MODIFY THIS FUNCTION #####
void setup_output_directory(char *block_folder, char *hash_folder)
{
    // Remove output directory
    pid_t pid = fork();
    if (pid == 0)
    {
        char *argv[] = {"rm", "-rf", "./output/", NULL};
        if (execvp(*argv, argv) < 0)
        {
            printf("ERROR: exec failed\n");
            exit(1);
        }
        exit(0);
    }
    else
    {
        wait(NULL);
    }

    sleep(1);

    // Creating output directory
    if (mkdir("output", 0777) < 0)
    {
        printf("ERROR: mkdir output failed\n");
        exit(1);
    }

    sleep(1);

    // Creating blocks directory
    if (mkdir(block_folder, 0777) < 0)
    {
        printf("ERROR: mkdir output/blocks failed\n");
        exit(1);
    }

    // Creating hashes directory
    if (mkdir(hash_folder, 0777) < 0)
    {
        printf("ERROR: mkdir output/hashes failed\n");
        exit(1);
    }
}