# Project 3: Parallel Image Processing

### Project Overview
Project 3 focuses on the use of multithreading to accelerate the process of rotating multiple images. By leveraging POSIX threads, mutex locks, and condition variables, this project aims to familiarize thread programming and synchronization.

### Objectives
- Implement a multi-threaded image rotation program.
- Create a fixed pool of worker threads.
- Use a shared request queue for managing image rotation tasks.
- Ensure proper synchronization and logging of requests.

### Components
1. **Processing Thread**
   - Traverses a specified directory for image files.
   - Adds the directory contents to a shared request queue.
   - Signals worker threads when no more data will be added to the queue.
   - Waits for acknowledgments from worker threads after all image rotations are completed.
   - Verifies that all images have been processed.

2. **Worker Threads**
   - Continuously monitor the shared request queue.
   - Retrieve requests and read image files.
   - Apply image rotation and save the rotated images.
   - Log each request to a file and the terminal.

### Key Concepts
- **Thread Pool:** A fixed number of worker threads created at the start of the program.
- **Request Queue Structure:** Contains file names and rotation angles, implemented as a queue or linked list of structs.
- **Image Rotation:** Uses `stbi_load` to read image data and `stbi_write_png` to save rotated images.
- **Synchronization:** Uses mutexes and condition variables to manage shared data access and thread termination.

### Directory Structure
- `include/`: Header files (`utils.h`, `image_rotation.h`)
- `lib/`: Library files (`utils.o`)
- `src/`: Source files (`image_rotation.c`)
- `img/`: Contains directories with images to be processed
- `expected/`: Expected output for image rotations (only 180-degree rotation angle)
- `Makefile`: Build instructions and testing commands
- `README.md`: Project information

### Compilation Instructions
To compile the project, use the following command:
```sh
make
```
To run the program use:
```sh
./image_rotation <input_dir> <output_dir> <number_threads> <rotation_angle>
```
To test the program with 10 worker threads use:
```sh
make test
```
