#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/hash.h"
#include "../include/utils.h"

char *output_file_folder = "output/final_submission/";

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: Inter Submission --> ./leaf_process <file_path> 0\n");
        printf("Usage: Final Submission --> ./leaf_process <file_path> <pipe_write_end>\n");
        return -1;
    }

    // Extract file path and pipe write end from command line arguments
    char *file_path = argv[1];
    char *pipe_write_end_str = argv[2];
    int pipe_write_end = atoi(pipe_write_end_str);

    // Create the hash of the given file
    char hash[50];
    hash_data_block(hash, file_path);

    // Construct string to write to pipe
    int required_length = strlen(file_path) + strlen(hash) + 3; // +2 for "|" and +1 for NULL terminator
    char *format = (char *)malloc(required_length * sizeof(char));
    if (!format)
    {
        perror("Failed to allocate memory for format");
        exit(-1);
    }
    snprintf(format, required_length, "%s|%s|", file_path, hash);
    
    //printf("In leaf, file path: %s, hash: %s\n", file_path, hash);
    // If pipe_write_end is 0, it's not the final submission
    if (pipe_write_end == 0)
    {
        //fprintf(stderr, "In leaf, file path: %s, hash: %s\n", file_path, hash);
        //fprintf(stderr, "This is NOT the final submission.\n");
        // Extract the file_name from file_path
        char *file_name = extract_filename(file_path);

        // Extract the root directory from file_path
        char *root_directory = extract_root_directory(file_path);

        // Get the location of the new file
        int new_location_length = strlen(output_file_folder) + strlen(root_directory) + strlen(file_name) + 5;
        char *new_location = (char *)malloc(new_location_length * sizeof(char));
        if (!new_location)
        {
            perror("Failed to allocate memory for new_location");
            free(format);
            exit(-1);
        }
        snprintf(new_location, new_location_length, "%s%s/%s", output_file_folder, root_directory, file_name);

        // Create and write to the file, then close the file
        //fprintf(stderr, "Trying to create file at: %s\n", new_location);
        FILE *file = fopen(new_location, "w");
        if (!file)
        {
            perror("Failed to open the file for writing");
            free(format);
            free(new_location);
            free(root_directory);
            exit(-1);
        }
        fwrite(format, 1, strlen(format), file);
        fclose(file);

        free(new_location);
        free(root_directory);
    }
    else
    {
        //fprintf(stderr, "This IS the final submission.\n");
        // Write the string to the pipe for the final submission
        write(pipe_write_end, format, strlen(format));
        //fflush(stdout);  // Flush data

        // Make sure to close the pipe write end
        close(pipe_write_end);
    }
    //fprintf(stderr, "Leaf process completed.\n");
    //clean up
    free(format);
    exit(0);
}
