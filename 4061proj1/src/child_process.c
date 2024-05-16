#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "hash.h"

#define PATH_MAX 1024

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        printf("Usage: ./child_process <blocks_folder> <hashes_folder> <N> <child_id>\n");
        return 1;
    }

    // getting N from third argument in argv
    int N = atoi(argv[3]);

    // start with parent and increment with each child process
    int ID = atoi(argv[4]);

    // check if it's a leaf
    if (ID >= (N - 1) && ID <= ((2 * N) - 2))
    {
        char read_file_path[PATH_MAX];

        // buffer to copy from ID
        char current[4];

        // compute the file to copy from
        // for example node 7 derives from 0 or ID - (N - 1)
        // and 14 derives from ID - (N - 1) or 7 if N is 8.
        int val = ID - (N - 1);

        // Creates character array for path
        sprintf(current, "%d", val);
        sprintf(read_file_path, "output/blocks/%s.txt", current);

        // Opens the read file for the hash using path array
        FILE *fd = fopen(read_file_path, "r");
        if (fd == NULL)
        {
            perror("Failed to open write file");
            return 1;
        }

        // Gets hash from function and puts it in character array
        char block_hash[SHA256_BLOCK_SIZE * 2 + 1];
        hash_data_block(block_hash, read_file_path);

        // Close file after use
        fclose(fd);
        size_t block_size = strlen(block_hash);

        // Creates character array for path
        char hash_path[PATH_MAX];
        sprintf(hash_path, "%s/%d.out", argv[2], ID);

        // Open file and write hash to it
        FILE *hash_file = fopen(hash_path, "w");
        if (!hash_file)
        {
            perror("Failed to open hash file for writing");
            return 1;
        }
        fwrite(block_hash, 1, block_size, hash_file);
        fclose(hash_file);
    }
    else
    {
        // creating IDs for children based on figure 4
        int left_child_ID = 2 * ID + 1;
        int right_child_ID = 2 * ID + 2;

        // Handle left child
        pid_t pid_left = fork();
        if (pid_left == 0)
        {
            // convert child id to character array
            char child_id[10];
            sprintf(child_id, "%d", left_child_ID);
            // Using execl to run child process with the same arguments and passing in the child ID
            execl("./child_process", "./child_process", argv[1], argv[2], argv[3], child_id, NULL);
        }

        // Handling right child, logic is the same
        pid_t pid_right = fork();
        if (pid_right == 0)
        {
            char child_id[10];
            sprintf(child_id, "%d", right_child_ID);
            execl("./child_process", "./child_process", argv[1], argv[2], argv[3], child_id, NULL);
        }

        // Waiting for both pids
        waitpid(pid_left, NULL, 0);
        waitpid(pid_right, NULL, 0);

        // Creating character arrays for path of hash files
        char left_path[PATH_MAX];
        char right_path[PATH_MAX];

        // Printing paths using hash folder and left/right child IDs
        sprintf(left_path, "%s/%d.out", argv[2], left_child_ID);
        sprintf(right_path, "%s/%d.out", argv[2], right_child_ID);

        // Creating character arrays for left and right hashes
        char left_hash[SHA256_BLOCK_SIZE * 2 + 1];
        char right_hash[SHA256_BLOCK_SIZE * 2 + 1];

        // Handling reading hash from left hash file, opens hash file using left path
        FILE *left_file = fopen(left_path, "r");
        // Reads hash from file into left_hash character array
        fread(left_hash, 1, sizeof(left_hash) - 1, left_file);
        if (!left_file)
        {
            perror("Failed to open left file for reading");
            return 1;
        }
        left_hash[sizeof(left_hash) - 1] = '\0'; // null terminate
        // Closes file
        fclose(left_file);

        // Handling reading hash from right hash file, logic is the same as left
        FILE *right_file = fopen(right_path, "r");
        if (!right_file)
        {
            perror("Failed to open right file for reading");
            return 1;
        }
        fread(right_hash, 1, sizeof(right_hash) - 1, right_file);
        right_hash[sizeof(right_hash) - 1] = '\0'; // null terminate
        fclose(right_file);

        // Creates a character array for the combined hashes and combines the concatenated hashes
        char combined[SHA256_BLOCK_SIZE * 2 + 1];
        compute_dual_hash(combined, left_hash, right_hash);

        // Creates a char array for output path for new hash
        char output_path[PATH_MAX];
        sprintf(output_path, "%s/%d.out", argv[2], ID);

        // creates file from output path and writes hash to it, closes when done
        FILE *output_file = fopen(output_path, "w");
        if (!output_file)
        {
            perror("Failed to open output file for writing");
            return 1;
        }
        fwrite(combined, 1, sizeof(combined) - 1, output_file);
        fclose(output_file);
    }
}
