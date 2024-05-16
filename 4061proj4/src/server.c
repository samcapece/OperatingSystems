#include "server.h"

#define PORT 5346
#define MAX_CLIENTS 5
#define STRSZ 100
#define BUFFER_SIZE 1024
const int PACKETSZ = sizeof(packet_t);  
pthread_t threads[MAX_CLIENTS]; // Threads delcared globally so they can be accessed by clientHandler
int loop = 1;
pthread_mutex_t thread_pool_mutex;
pthread_cond_t thread_available_cond = PTHREAD_COND_INITIALIZER;
int numberOfActiveThreads = 0;

void ctrlcHandler(int signo) {
    fprintf(stdout, "Interrupt signal received, shutting down...\n");
    fflush(stdout);
    loop = 0;
}


// Deserialize data
packet_t *deserializeData(char *serializedData) {
    packet_t *packet = (packet_t *)malloc(sizeof(packet_t));
    memset(packet, 0, PACKETSZ);
    memcpy(packet, serializedData, PACKETSZ);
    return packet;
}

void *clientHandler(void *socket) {
    //Cast socket to int
    int client_socket = *((int *)socket);
    free(socket);
    printf("Clienthandler socket created\n");

    while(1){
        char packet_buffer[PACKETSZ];
        memset(packet_buffer, 0, PACKETSZ);

        

        // Receive packets from the client
        ssize_t packet_bytes_received = recv(client_socket, packet_buffer, PACKETSZ, 0);
        if (packet_bytes_received < 0) {
            perror("Error receiving packet");
            // Handle error appropriately, possibly close the connection and return
            break;
        }
        else if (packet_bytes_received == 0)
        {
            printf("Connection closed by client\n");
            // Connection closed by client
            break;
        }


        printf("packet recieved from client confirmation: %zd\n", packet_bytes_received);

        // Deserialize recieved data
        packet_t *packet = deserializeData(packet_buffer);
        if (packet == NULL) {
            break;
        }


        printf("Packet operation: %d\n", packet->operation);
        printf("Flags operation: %d\n", packet->flags);

        // Process the packet based on its operation type
        if (packet->operation == IMG_OP_ROTATE) {
            printf("Rotate Image Operation Received\n");

            // Additional image processing logic should be added here in final submission
            // Send acknowledgment back to client (could be implemented later)
        } else if (packet->operation == IMG_OP_EXIT) {
            printf("Terminate connection operation Received\n");
            break;
        }
        char buffer[BUFFER_SIZE];

        printf("Size of message: %d\n", packet->size);

        ssize_t bytes_received = recv(client_socket, buffer, packet->size, 0);

        if (bytes_received < 0) {
            perror("Receive failed");
            break;
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
            break;
        } else {
            char received_value = buffer[0];
            printf("Received char value: %c\n", received_value);
        }

        int width, height, bpp;

        // Process the image data based on the set of flags

        uint8_t *image_result = (uint8_t *)buffer;
        image_result = stbi_load_from_memory(image_result, packet->size, &width, &height, &bpp, CHANNEL_NUM);

        printf("Image result width: %d\n", width);
        printf("Image result height: %d\n", height);
        printf("Image result bpp: %d\n", bpp);

        if (image_result == NULL) {
            fprintf(stderr, "Error loading image: %s\n", stbi_failure_reason());
            packet->flags = IMG_OP_NAK;
            send(client_socket, "Error loading image", strlen("Error loading image"), 0);
            free(image_result);
            break;
        }

        // Allocate memory for the result matrix (widths)
        uint8_t **result_matrix = (uint8_t **)malloc(sizeof(uint8_t *) * width);

        // Allocate memory to store the original image data in array format (widths)
        uint8_t **img_matrix = (uint8_t **)malloc(sizeof(uint8_t *) * width);

        // Allocate heights of the result and image matrices
        for (int i = 0; i < width; i++) {
            result_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
            img_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
        }

        // Fill in the image matrix with data from the original image
        linear_to_image(image_result, img_matrix, width, height);

        // Figure out which function we will call.
        if (packet->flags == IMG_FLAG_ROTATE_180) {
            flip_left_to_right(img_matrix, result_matrix, width, height);
        }
        else if (packet->flags == IMG_FLAG_ROTATE_270) {
            flip_upside_down(img_matrix, result_matrix, width, height);
        }

        // Convert the result_matrix data to a byte array
        uint8_t *result_data = (uint8_t *)malloc(width * height);
        int k = 0;
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                result_data[k++] = result_matrix[i][j];
            }
        }

        int output_size;
        unsigned char* encoded_image = stbi_write_png_to_mem(result_data, 0, width, height, CHANNEL_NUM, &output_size);
    
        packet_t ackpacket;
        ackpacket.operation = IMG_OP_ACK; // Set to appropriate acknowledgment operation
        ackpacket.size = output_size; // Use the same size as the received packet for the image data

        // Send acknowledgment packet
        int bytes_sent = send(client_socket, &ackpacket, sizeof(ackpacket), 0);    if (bytes_sent == -1) {
            fprintf(stderr, "Error sending acknowledgment: %s\n", strerror(errno));
        } else {
            printf("Acknowledgment sent successfully\n");
        }

        bytes_sent = send(client_socket, encoded_image, output_size, 0);
        if (bytes_sent == -1) {
            fprintf(stderr, "Error sending image data: %s\n", strerror(errno));
        } else if (bytes_sent != output_size) {
            fprintf(stderr, "Incomplete image data sent. Sent: %d bytes, Expected: %d bytes\n", bytes_sent, width * height);
        } else {
            printf("Image data sent successfully\n");
        }

        // Free allocated memory
        stbi_image_free(image_result);

        // free allocated memory for matrices
        for (int i = 0; i < width; i++) {
            free(result_matrix[i]);
            free(img_matrix[i]);
        }
        free(result_matrix);
        free(img_matrix);
        free(result_data);
        free(packet);
    }

    pthread_mutex_lock(&thread_pool_mutex);
    numberOfActiveThreads--;
    pthread_cond_signal(&thread_available_cond);
    pthread_mutex_unlock(&thread_pool_mutex);

    close(client_socket);
    return NULL;
}

int main(int argc, char* argv[]) {
    // Setting up signal for gracefully exiting while loop
    struct sigaction action;
    action.sa_handler = ctrlcHandler;
    action.sa_flags = 0;

    if ((sigemptyset(&action.sa_mask) == -1) || // clear set
        (sigaddset(&action.sa_mask, SIGINT) == -1) || // add SIGINT to mask
        (sigaction(SIGINT, &action, NULL) == -1)) { // set SIGINT handler
        perror("Failed to set SIGTSTP");
        return -1;
    }

    // Initializing mutex
    if (pthread_mutex_init(&thread_pool_mutex, NULL) != 0) {
        perror("Mutex init has failed");
        return 1; // Or handle error appropriately
    }


    // Creating socket file descriptor
    int sockfd, conn_fd;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("Socket created\n");

    // Bind the socket to the port

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr)); //replaced bzero w/ memset
    servaddr.sin_family = AF_INET;                                             // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);                              // Listen to any of the network interface (INADDR_ANY)
    servaddr.sin_port = htons(PORT);                                           // Port number

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Bind successful\n");

    if (listen(sockfd, MAX_CLIENTS) == -1) {
        perror("listen error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("Listen successful\n");

    // Accept connections and create the client handling threads
    while (loop) {
        fprintf(stdout, "Server: ");
        struct sockaddr_in clientaddr;
        socklen_t clientaddr_len = sizeof(clientaddr);

        conn_fd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientaddr_len);
        if (conn_fd == -1) {
            perror("accept error");
            continue;
        }

        printf("New Client connected\n");

        pthread_mutex_lock(&thread_pool_mutex);

        while (numberOfActiveThreads >= MAX_CLIENTS) {
            pthread_cond_wait(&thread_available_cond, &thread_pool_mutex);
        }

        int *client_sock = malloc(sizeof(int));
        *client_sock = conn_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (pthread_create(&threads[i], NULL, clientHandler, client_sock) == 0) {
                numberOfActiveThreads++;
                break;
            }
        }

        pthread_mutex_unlock(&thread_pool_mutex);
    }
    fprintf(stdout, "Gracefully exiting...\n");

    // Wait for all active threads to finish
    for (int j = 0; j < MAX_CLIENTS; j++) {
        pthread_join(threads[j], NULL);
    }

    pthread_mutex_destroy(&thread_pool_mutex);
    pthread_cond_destroy(&thread_available_cond);

    close(sockfd);
    return 0;
}
