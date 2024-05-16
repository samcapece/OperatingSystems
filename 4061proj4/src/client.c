#include <string.h>
#include "client.h"

#define PORT 5346
#define BUFFER_SIZE 1024
const int PACKETSZ = sizeof(packet_t);

// Global request queue and queue length variables
request_t global_request_queue[MAX_QUEUE_LEN];
int queue_length = 0;

// Function to add a file to the request queue
void add_to_request_queue(const char *file_name, int rotation_angle) {
    if (queue_length < MAX_QUEUE_LEN) { // Check if queue can be added to
        global_request_queue[queue_length].rotation_angle = rotation_angle; // Set rotation angle
        global_request_queue[queue_length].file_name = strdup(file_name); // Set file name, strdup freed in main
        queue_length++; // Increment queue
    }
}

// Serialize packet
char *serializePacket(packet_t *packet){
    char *serializedData = (char *)malloc(PACKETSZ);
    memset(serializedData, 0, PACKETSZ);
    memcpy(serializedData, packet, PACKETSZ);
    return serializedData;
}

// Send a file to the server
int send_file(int socket, const char *filename, int rotation_angle) {
    // Open the file
    FILE *file_pointer = fopen(filename, "rb");
    if (file_pointer == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    // Get the file size
    fseek(file_pointer, 0, SEEK_END);     // Move file pointer to end
    long file_size = ftell(file_pointer); // Get current position (which is file size)
    fseek(file_pointer, 0, SEEK_SET);     // Move file pointer back to start

    // Set up the request packet for the server and send it
    packet_t packet;
    packet.operation = IMG_OP_ROTATE;
    // Set flags based on the rotation angle using if-else statement
    if (rotation_angle == 180) {
        packet.flags = IMG_FLAG_ROTATE_180;
    } else if (rotation_angle == 270) {
        packet.flags = IMG_FLAG_ROTATE_270;
    } else {
        packet.flags = 0; // No rotation flag if angle is neither 180 nor 270
    }

    printf("Packet operation: %d\n", packet.operation);
    printf("Flags operation: %d\n", packet.flags);
    printf("Image size: %ld bytes\n", file_size); // Display the image size

    // Read data from the file and send it over the socket
    char image_data[BUFFER_SIZE];

    size_t bytes_read = fread(image_data, 1, file_size, file_pointer);
    if (bytes_read != file_size) {
        perror("Error reading file");
        fclose(file_pointer);
        return -1;
    }
    packet.size = (int)file_size;
    // Send image data to the server

    // Serialize the packet
    char *serializedData = serializePacket(&packet);

    // Send message to server
    int ret = send(socket, serializedData, PACKETSZ, 0); 
    if (ret == -1) {
        perror("send error");
        free(serializedData);
        fclose(file_pointer);
        return -1;
    }

    printf("Packet successfully sent\n");

    free(serializedData);
    int ret2 = send(socket, image_data, file_size, 0);
    if (ret2 == -1) {
        perror("Error sending data");
        fclose(file_pointer);
        return -1;
    }

    printf("Image send confirmation: %d\n", ret2);

    fclose(file_pointer);
    // printf("Total bytes sent: %zu\n", total_bytes_sent); // Optional: Display total bytes sent
    return 0;
}


int receive_file(int socket, const char *filename) {
    // Receive response packet
    char packet_buffer[PACKETSZ];
    memset(packet_buffer, 0, PACKETSZ);

    ssize_t packet_bytes_received = recv(socket, packet_buffer, PACKETSZ, 0);
    if (packet_bytes_received < 0) {
        perror("Error receiving packet");
        return -1;
    } else if (packet_bytes_received == 0) {
        printf("Connection closed by server\n");
        return -1;
    }

    packet_t packet;
    memcpy(&packet, packet_buffer, sizeof(packet_t));

    // Check for ACK flag
    if (packet.operation != IMG_OP_ACK) {
        fprintf(stderr, "Received NAK from server or invalid operation\n");
        return -1;
    }

    // Allocate buffer for the image data
    unsigned char *image_data = malloc(packet.size);
    if (image_data == NULL) {
        fprintf(stderr, "Failed to allocate memory for image data\n");
        return -1;
    }

    // Receive the file data
    ssize_t bytes_received = recv(socket, image_data, packet.size, 0);
    if (bytes_received < 0) {
        perror("Error receiving image data");
        free(image_data);
        return -1;
    } else if (bytes_received == 0) {
        printf("Connection closed by server\n");
        free(image_data);
        return -1;
    }

    printf("value of recieved bytes: %zd \n", bytes_received); 

    // Open the file for writing
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Error opening file for writing");
        free(image_data);
        return -1;
    }

    // Write the data to the file
    size_t bytes_written = fwrite(image_data, 1, bytes_received, file);
    if (bytes_written != bytes_received) {
        fprintf(stderr, "Error writing image data to file\n");
        fclose(file);
        free(image_data);
        return -1;
    }

    // Close the file and free resources
    fclose(file);
    free(image_data);

    printf("File received and saved successfully: %s\n", filename);

    return 0;
}

int send_exit_signal(int socket) {
    packet_t packet;
    packet.operation = IMG_OP_EXIT;
    packet.flags = 0;
    packet.size = 0;

    char *serializedData = serializePacket(&packet);
    int ret = send(socket, serializedData, PACKETSZ, 0);
    free(serializedData);

    if (ret == -1) {
        perror("Error sending exit signal");
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if(argc != 4){
        fprintf(stderr, "Usage: %s File_Path_to_images File_Path_to_output_dir Rotation_angle\n", argv[0]);
        return 1;
    }
    
    // Convert rotation angle
    int rotation_angle = atoi(argv[3]);

    // Set up socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // create socket to establish connection

    // Connect the socket
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;                     // IPv4
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // server IP, since the server is on same machine, use localhost IP
    servaddr.sin_port = htons(PORT);                   // Port the server is listening on

    // Connect to server
    int ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); // establish connection to server
    if (ret == -1) {
        perror("connect error");
        return 1;
    }

    // Read the directory for all the images to rotate

    // stores information about directory entires
    struct dirent *entry;

    char *dir = argv[1];
    DIR *directory = opendir(dir);
    if (!directory) {
        perror("Unable to open directory");
        return 1;
    }

    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_name[0] != '.' && strstr(entry->d_name, ".png")) {
            // Construct full file path
            char filepath[PATH_MAX];
            snprintf(filepath, sizeof(filepath), "%s/%s", argv[1], entry->d_name);
            // Add file to the request queue
            add_to_request_queue(filepath, rotation_angle);
        }
    }
    closedir(directory); // Close the directory

    // Process and receive each file in the request queue
    char output_path[PATH_MAX];
    for (int i = 0; i < queue_length; i++) {
        if (send_file(sockfd, global_request_queue[i].file_name, global_request_queue[i].rotation_angle) < 0) {
            fprintf(stderr, "Error sending file: %s\n", global_request_queue[i].file_name);
        } else {
            snprintf(output_path, sizeof(output_path), "%s%s", argv[2], strrchr(global_request_queue[i].file_name, '/'));
            if (receive_file(sockfd, output_path) < 0) {
                fprintf(stderr, "Error receiving processed file: %s\n", output_path);
            }
        }
        free(global_request_queue[i].file_name);
    }

    // Check that the request was acknowledged

    // Receive the processed image and save it in the output dir

    // Terminate the connection once all images have been processed
    if (send_exit_signal(sockfd) < 0) {
        fprintf(stderr, "Error sending exit signal to server\n");
        close(sockfd);
        return 1;
    }


    // Release any resources

    close(sockfd); // Close the socket
    return 0;
}
