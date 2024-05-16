# Project 4: Client-Server Image Processing

### Project Overview
Project 4 involves creating a client-server application for image processing using socket programming. The goal is to develop a server that handles multiple client connections concurrently, processes image rotation requests, and sends back the processed images. This project emphasizes understanding and implementing TCP sockets and client-server communication.

### Objectives
- Implement a multi-threaded server to handle image processing requests from multiple clients.
- Develop a client program to send images to the server for processing.
- Use socket programming to establish and manage TCP connections between clients and the server.

### Components
1. **Server Program**
   - Initializes a TCP socket and listens for incoming connections on a specific port.
   - Accepts client connections and creates a new thread for each client to handle requests.

2. **Client Handling Thread**
   - Receives image rotation requests from the client.
   - Processes the image (rotate 180 or 270 degrees) and sends the processed image back to the client.
   - Closes the connection when the client terminates the session.

3. **Client Program**
   - Takes an input directory as a command-line argument.
   - Connects to the server and sends image rotation requests.
   - Receives and saves the processed images from the server.

### Communication Protocol
- **Packet Format:**
  - `Operation (int)`
  - `Flags (int)`
  - `Image Size (int)`
  - `Checksum (Char, 32 bytes, extra credit)`
  - `Image Data`

- **Operations:**
  - `IMG_OP_ACK (1)`: Acknowledgement.
  - `IMG_OP_NAK (2)`: Negative acknowledgement.
  - `IMG_OP_ROTATE (3)`: Request for image rotation.
  - `IMG_OP_EXIT (4)`: Termination of connection.

- **Flags:**
  - `IMG_FLAG_ROTATE_180 (1)`: 180-degree rotation.
  - `IMG_FLAG_ROTATE_270 (2)`: 270-degree rotation.
  - `IMG_FLAG_ENCRYPTED (3)`: Message is encrypted.
  - `IMG_FLAG_CHECKSUM (4)`: Packet contains a checksum for the image.

### Directory Structure
- `include/`: Header files (`utils.h`, `server.h`, `client.h`)
- `lib/`: Library files (`utils.o`)
- `src/`: Source files (`server.c`, `client.c`)
- `img/`: Contains multiple directories with different images
- `expected/`: Expected output
- `Makefile`: Build instructions and testing commands

### Compilation Instructions
To compile the server, client, or both:
```sh
# Compile the server
make server

# Compile the client
make client

# Compile both server and client
make all
```
To run the server and client:
```sh
./server
# In separate terminal
./client <input_dir> <output_dir> <rotate_angle>
```
To clean out the compiled server, client, and reset input/output directory:
```sh
make clean
```
