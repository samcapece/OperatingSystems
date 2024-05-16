# proj4


We have written confirmation from the professor that we were allowed a 2-day extension

- Project group number
18

- Group member names and x500s

Alexander Tatley, tatle010 

Cole Schmidt, schm5346 

Samuel Capece, capec016

- The name of the CSELabs computer that you tested your code on

login01.cselabs.umn.edu

- Any changes you made to the Makefile or existing files that would affect
grading

none

- Plan outlining individual contributions for each member of your group

Alex - debugging

Cole - image processing for client handler, send_file, I/O between server and client, and debugging

Samuel - recieve file, bugfixed server and client code, rewrote server main and client_handler to process multiple images, and added functionality for switching clients using mutexes

- Plan on how to construct the client handling thread and package sending


ok, the clienthandler will have to discern within the packet of whatever it's IMG_OP_ACK (1), IMG_OP_NAK (2), IMG_OG_ROTATE (3), or IMG_OP_EXIT (4).

It should do this using if statements and discerning the content of the packet it recieved from a specific client.

If it's 4 for an operation, send a packet to the client and terminate the connection i.e. the socket. 4 should only be used from the clienthandler to send to a client in when it finished it's image processing.

2 should be used when there's an error or incoherent contents in the packet that the client handler received from a client

3 should only be used from client to server in send_file() in client.c.


when the client is recieving any requests, it'll have to do so with a for loop like the used in file sending. Client should be in a loop like in server so that the file receiving doesn't get included with cleaning resources but a part of the  process

The client handler should discern the degree of image rotation based on the packet.

There'll be a packet sent to the client handler for the rotations, flags, and including a unique hash for a hypothetical image. There'll be another one for the images, ideally encypted using the SHA256 function and discernable using a hash value. 

After the image has been processed by the client handler, it'll be sent back to the client using the SHA256 encryption and with it's own checksum.

The checksum that'll be derived from mixing the hash value from the original image (left) and the processed image (right) as it's own unqiue hash that'll be sent back to the client. 

Moreover, the next image will have to take the hash value from the preceding image for it's own unique hash value and use that to create a checksum for the nest processed image.



It's only different in the case that we couldn't submit encryption and the checksum in time

// partially lifted from file

send_file(rotationAngle) {
    packet.operation = IMG_OP_ROTATE

    if (rotationAngle == 180) {
        packet.flags = IMG_FLAG_ROTATE_180;
    } else if (rotation_angle == 270) {
        packet.flags = IMG_FLAG_ROTATE_180;
    } else {
        packet.flags = 0; // No rotation flag if angle is neither 180 nor 270
    }

    ...

    // Serialize the packet
    char *serializedData = serializePacket(&packet);

    // Send message to server
    int ret = send(socket, serializedData, PACKETSZ, 0); 
    if (ret == -1) {
        perror("send error");
        free(serializedData);
        close(file);
        return -1;
    }
}

recieve_file() {
    char packet_buffer[PACKETSIZE];
    memorySet(packet_buffer, NOVALUE, PACKETSIZE);

    ssize_t packet_bytes_received = recieveMessage(socket, packet_buffer, PACKETSIZE, NOFLAG);
    if (packet_bytes_received < 0) {
        perror("Error receiving packet");
        return -1;
    } else if (packet_bytes_received == 0) {
        printf("Connection closed by server\n");
        return -1;
    }
    
    packet_t packet;
    memcpy(&packet, packet_buffer, sizeof(packet_t));

    ... (allocate memory for image)

    // Check for ACK flag
    if (packet.operation != IMG_OP_ACK) {
        fprintf(stderr, "Received NAK from server or invalid operation\n");
        return -1;
    }
}

clientHandler(socket) {
    //Cast socket to int
    int client_socket = *((int *)socket);
    free(socket);
    printf("Clienthandler socket created\n");
    while(1) {
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

            ... (recieving and processing image data)

            printf("Packet operation: %d\n", packet->operation);
            printf("Flags operation: %d\n", packet->flags);

            // Process the packet based on its operation type
            if (packet->operation == IMG_OP_ROTATE) {
                printf("Rotate Image Operation Received\n");
            } else if (packet->operation == IMG_OP_EXIT) {
                printf("Terminate connection operation Received\n");
                break;
            }

            packet_t ackpacket;
            ackpacket.operation = IMG_OP_ACK; // Set to appropriate acknowledgment operation
            ackpacket.size = output_size; // Use the same size as the received packet for the image data

            // Send acknowledgment packet
            int bytes_sent = send(client_socket, &ackpacket, sizeof(ackpacket), 0);    if (bytes_sent == -1) {
                fprintf(stderr, "Error sending acknowledgment: %s\n", strerror(errno));
            } else {
                printf("Acknowledgment sent successfully\n");
            }
    }
}