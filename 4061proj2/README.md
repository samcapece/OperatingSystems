# Project 2: Data Deduplication

## Project Description
This project focuses on implementing a data deduplication system using the concepts of process management, file operations, and inter-process communication (IPC) in a Unix/Linux environment. The goal is to enhance storage efficiency by eliminating duplicate copies of data within a file system.

## Project Goals
- **Data Deduplication**: Efficiently identify and eliminate duplicate data to optimize storage utilization.
- **Process Management**: Utilize Unix/Linux system calls like `fork()`, `exec()`, `wait()`, and others to manage processes.
- **Inter-process Communication**: Implement a structured communication system using pipes to handle data flow between processes.

## System Requirements
- Unix/Linux environment
- GCC compiler

## Features
- **Hierarchical Process Tree**: Build a process tree where each process represents a file or directory within the filesystem hierarchy.
- **File Hashing**: Calculate hash values for files to identify duplicates.
- **Symbolic Links**: Replace duplicate files with symbolic links to the original file to save space.

## Setup and Compilation
- Clone the repository: `git clone [repository URL]`
- Navigate to the project directory: `cd [project directory]`
- Compile the project using:
  ```bash
  make all
  ```

  ## Usage
  To run the dedeuplication system:
  ```bash
  ./deduplication root_directory
  ```
  Where 'root_directory' is the path to the directory you want to deduplicate

  ## Testing
  To run the provided tests:
  ```bash
  make test
  ```
