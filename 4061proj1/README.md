# Project 1: Merkle Tree Implementation

## Project Description
This project involves implementing a Merkle Tree, a cryptographic hash tree used for efficient data verification across different systems. The implementation utilizes the `fork()`, `exec()`, and `wait()` functions to parallelize the building of the tree, exploring practical applications of process management and file I/O operations in a Unix/Linux environment.

## Project Goals
- **Efficient Data Verification**: Implement Merkle trees to ensure data integrity across distributed systems.
- **Parallel Processing**: Utilize Unix/Linux process management to parallelize tree construction.
- **File Handling**: Perform operations on file data to construct hash trees.

## System Requirements
- Unix/Linux environment
- GCC compiler

## Features
- **Merkle Tree Construction**: Build a binary Merkle tree using parallel processes.
- **Data Integrity Verification**: Ensure the integrity of data using cryptographic hash functions.

## Setup and Compilation
- Clone the repository: `git clone [repository URL]`
- Navigate to the project directory: `cd [project directory]`
- Compile the project: `make all`

## Usage
To run the Merkle Tree program:
```bash
./merkle input_file.txt N
```
Where input_file.txt is the file containing the data to be hashed and N is the number of data blocks to split the input file into.

To run the tests use:
```bash
make test
```
