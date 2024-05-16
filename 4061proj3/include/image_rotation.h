#ifndef IMAGE_ROTATION_H_
#define IMAGE_ROTATION_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <stdint.h>
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define CHANNEL_NUM 1

#include "stb_image.h"
#include "stb_image_write.h"



/********************* [ Helpful Macro Definitions ] **********************/
#define BUFF_SIZE 1024 
#define LOG_FILE_NAME "request_log"               //Standardized log file name
#define INVALID -1                                  //Reusable int for marking things as invalid or incorrect
#define MAX_THREADS 100                             //Maximum number of threads
#define MAX_QUEUE_LEN 100                           //Maximum queue length



/********************* [ Helpful Typedefs        ] ************************/

typedef struct request
{
    char* filename;      // Filename for the image to be processed
    int rotation_angle;  // Rotation angle for the image (180 or 270)
    int requestNum;      // Unique identifier for the request
} request_t;

typedef struct request_queue {
    request_t* queue;       // Pointer to the array of requests
    int front;              // Index of the front of the queue
    int rear;               // Index of the rear of the queue
    int count;              // Number of items in the queue
    int capacity;           // Maximum number of items that the queue can hold
    pthread_mutex_t lock;   // Mutex for protecting the queue
    pthread_cond_t can_add; // Condition variable for signaling that items can be added to the queue
    pthread_cond_t can_assign; // Condition variable for signaling that items can be removed from the queue
    bool no_more_requests;
} request_queue_t;

typedef struct processing_args
{
    char* input_dir;         // Directory containing the images to be processed
    char* output_dir;        // Directory where the processed images should be saved
    request_queue_t* queue;  // Pointer to the shared request queue
    int rotation_angle;     // Rotation angle to apply to the image
} processing_args_t;

typedef struct {
    int thread_id;          // Thread id for each individual worker
    int request_count;      // Keeps track of number of requests for each thread
    char* input_dir;        // The input directory;
    char* output_dir;       // The output directory
    request_queue_t* queue; // Pointer to the global request queue
    FILE* log_file;         // Log file
} worker_args_t;

/********************* [ Function Prototypes       ] **********************/
void *processing(void *args); 
void * worker(void *args); 
void log_pretty_print(FILE* to_write, int threadId, int requestNumber, char * file_name);
void initialize_request_queue(request_queue_t *queue, int max_size);

#endif
