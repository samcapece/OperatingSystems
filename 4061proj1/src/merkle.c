#include "utils.h"
#include "print_tree.h"
#define PATH_MAX 1024

// ##### DO NOT MODIFY THESE VARIABLES #####
char *blocks_folder = "output/blocks";
char *hashes_folder = "output/hashes";
char *visualization_file = "output/visualization.txt";

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        // N is the number of data blocks to split <file> into (should be a power of 2)
        // N will also be the number of leaf nodes in the merkle tree
        printf("Usage: ./merkle <file> <N>\n");
        return 1;
    }

    printf("Input file: %s\n", argv[1]);
    printf("second arg: %s\n", argv[2]);

    // Reads n from args
    int n = atoi(argv[2]);

    printf("bytes: %d\n", n);

    // Reads input file from args
    char *input_file = argv[1];

    // ##### DO NOT REMOVE #####
    setup_output_directory(blocks_folder, hashes_folder);

    // Partitions file data from input file into blocks
    partition_file_data(input_file, n, blocks_folder);

    // Starts the recursive merkle tree computation by spawning first child process (root), opens first file
    char root_path[50] = "output/hashes/0.out";
    FILE *root_file = fopen(root_path, "w");
    if (root_file == NULL)
    {
        perror("Failed to open write file");
        return 1;
    }
    pid_t root = fork();

    if (root == 0)
    { // in child process
        // passes in root id as 0, uses execl to start first child process
        char root_id[2];
        sprintf(root_id, "%d", 0);
        execl("./child_process", "child_process", blocks_folder, hashes_folder, argv[2], root_id, NULL);
    }
    wait(NULL); // wait for the root node to process.

// ##### DO NOT REMOVE #####
#ifndef TEST_INTERMEDIATE
                // Visually display the merkle tree using the output in the hashes_folder
    print_merkle_tree(visualization_file, hashes_folder, n);
#endif
    fclose(root_file); // once done waiting, execute print_merkle_tree();
    // Closes file
    return 0;
}