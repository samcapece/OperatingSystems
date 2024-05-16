#include "image_rotation.h"
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>

//Global integer to indicate the length of the queue??
//Global integer to indicate the number of worker threads
//Global file pointer for writing to log file in worker??
//Structure for queue elements
//Might be helpful to track the ID's of your threads in a global array
//What kind of locks will you need to make everything thread safe? [Hint you need multiple]
//What kind of CVs will you need  (i.e. queue full, queue empty) [Hint you need multiple]
//How will you track the requests globally between threads? How will you ensure this is thread safe?
//How will you track which index in the request queue to remove next?
//How will you update and utilize the current number of requests in the request queue?
//How will you track the p_thread's that you create for workers?
//How will you know where to insert the next request received into the request queue?

//Global file pointer for writing to log file
FILE* logFile;

//Global mutex for logging
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

//Global number of worker threads
int numWorkers;

//Global request queue variable
request_queue_t global_request_queue;

//Initializes request queue, called in main
void initialize_request_queue(request_queue_t *queue, int max_size) {
    queue->queue = (request_t *)malloc(sizeof(request_t) * max_size);
    queue->front = 0;
    queue->rear = -1;  // Back is -1 when queue is empty
    queue->count = 0;
    queue->capacity = max_size;
    queue->no_more_requests = false;
    pthread_mutex_init(&(queue->lock), NULL);
    pthread_cond_init(&(queue->can_add), NULL);
    pthread_cond_init(&(queue->can_assign), NULL);
}


/*
    The Function takes:
    to_write: A file pointer of where to write the logs. 
    requestNumber: the request number that the thread just finished.
    file_name: the name of the file that just got processed. 

    The function output: 
    it should output the threadId, requestNumber, file_name into the logfile and stdout.
*/
void log_pretty_print(FILE* to_write, int threadId, int requestNumber, char * file_name){
    // Make sure only one thread writes to the log at a time
    pthread_mutex_lock(&log_mutex); 

    // Take directory into account
    char location[50] = "./img/";
    strcat(location, file_name);

    // Format the log message
    fprintf(to_write, "[%d][%d][%s]\n", threadId, requestNumber, location);
    fflush(to_write); // Flush the stream to make sure the data is written

    // Print to stdout
    printf("[%d][%d][%s]\n", threadId, requestNumber, location);

    pthread_mutex_unlock(&log_mutex);
}


/*

    1: The processing function takes a void* argument called args. It is expected to be a pointer to a structure processing_args_t 
    that contains information necessary for processing.

    2: The processing thread need to traverse a given dictionary and add its files into the shared queue while maintaining synchronization using lock and unlock. 

    3: The processing thread should pthread_cond_signal/broadcast once it finish the traversing to wake the worker up from their wait.

    4: The processing thread will block(pthread_cond_wait) for a condition variable until the workers are done with the processing of the requests and the queue is empty.

    5: The processing thread will cross check if the condition from step 4 is met and it will signal to the worker to exit and it will exit.

*/

void *processing(void *args) {
    //stores information about directory entires
    struct dirent *entry;

    // get pointer from args
    processing_args_t *arguments = (processing_args_t *)args;

    // open the directory from args
    DIR *directory = opendir(arguments->input_dir);

    // error checking
    if (directory == NULL) {
        perror("Unable to open directory");
        return NULL;
    }

    //Reads entires from the opened directory until none left
    while ((entry = readdir(directory)) != NULL) {

        if (entry->d_name[0] == '.') continue; // Skip the . and .. directories

        //Checking if file is a PNG
        const char *dot = strrchr(entry->d_name, '.');
        if (dot && strcmp(dot, ".png") == 0) { //Check if dot is found and extension is .png
            pthread_mutex_lock(&(arguments->queue->lock)); //Locks request queue

            // Wait until there is space in the queue
            while (arguments->queue->count >= arguments->queue->capacity) { //While queue is at capacity
                pthread_cond_wait(&(arguments->queue->can_add), &(arguments->queue->lock)); //If we can add to queue we unlock
            }

            // Add to the queue
            int new_rear = (arguments->queue->rear + 1) % arguments->queue->capacity; //Variable for the new end of the queue
            arguments->queue->queue[new_rear].filename = strdup(entry->d_name); // Allocate and copy filename, must be freed later in worker thread
            arguments->queue->queue[new_rear].rotation_angle = arguments->rotation_angle; //Sets rotation angle from args to the queue
            arguments->queue->rear = new_rear; //Updates rear to the new rear position
            arguments->queue->count++; //Increments items in the queue

            pthread_cond_signal(&(arguments->queue->can_assign)); // Signal that there is a new item in the queue
            pthread_mutex_unlock(&(arguments->queue->lock)); //Signals that a new item has been added to the queue
        }
    }

    // signal to queue that it finished transversing
    pthread_mutex_lock(&(arguments->queue->lock)); //Locks mutex 
    arguments->queue->no_more_requests = true; // Set a flag that no more requests will be added
    pthread_cond_broadcast(&(arguments->queue->can_assign)); // Wake up all workers to check queue
    pthread_mutex_unlock(&(arguments->queue->lock)); //Unlocks mutex after setting flag and broadcasting

    closedir(directory); //Close directory
    return NULL;
}

/*
    1: The worker threads takes an int ID as a parameter

    2: The Worker thread will block(pthread_cond_wait) for a condition variable that there is a requests in the queue. 

    3: The Worker threads will also block(pthread_cond_wait) once the queue is empty and wait for a signal to either exit or do work.

    4: The Worker thread will processes request from the queue while maintaining synchronization using lock and unlock. 

    5: The worker thread will write the data back to the given output dir as passed in main. 

    6:  The Worker thread will log the request from the queue while maintaining synchronization using lock and unlock.  

    8: Hint the worker thread should be in a While(1) loop since a worker thread can process multiple requests and It will have two while loops in total
        that is just a recommendation feel free to implement it your way :) 
    9: You may need different lock depending on the job.  

*/


void * worker(void *args) {
    worker_args_t *arguments = (worker_args_t *)args;
    int thread_id = arguments->thread_id; // Set the thread ID
    char* input_dir = arguments->input_dir; // Set the input directory
    char* output_dir = arguments->output_dir; // Set the output directory
    request_queue_t* queue = arguments->queue; // Pointer to global request queue
    


    while (true) { // Infinite loop, exits when no more requests
        pthread_mutex_lock(&(queue->lock)); // Locks request queue

        while (queue->count == 0) { // Checks if there are requests in queue
            if (queue->no_more_requests) {
                pthread_mutex_unlock(&(queue->lock));
                printf("Thread %d exiting as no more requests and queue is empty.\n", thread_id);
                pthread_exit(NULL); // Exit if no more requests and queue is empty
            }
            printf("Thread %d waiting for requests...\n", thread_id);
            pthread_cond_wait(&(queue->can_assign), &(queue->lock)); // If more requests, waits for request
            printf("Thread %d woke up from waiting.\n", thread_id);
        }

        
        // Dequeue and process request, increment request count for logging...
        printf("Thread %d processing a request...\n", thread_id);
        arguments->request_count++;

        // Dequeue the request
        request_t request = queue->queue[queue->front]; // Assigns current request
        queue->front = (queue->front + 1) % queue->capacity; // Sets new front of the queue
        queue->count--; // Decrements the queue

        // Log the request
        log_pretty_print(arguments->log_file, arguments->thread_id, arguments->request_count, request.filename);

        pthread_cond_signal(&(queue->can_add));  // Signal that space is available in queue
        printf("Signal sent: A new request is available.\n");

        pthread_mutex_unlock(&(queue->lock)); // Unlocks the queue

        /*
            Stbi_load takes:
                A file name, int pointer for width, height, and bpp

        */

        // Creating file path for the input file
        char input_filepath[1024];
        snprintf(input_filepath, sizeof(input_filepath), "%s/%s", input_dir, request.filename);
        
        int width, height, bpp;

        // Loading the image file
        uint8_t* image_result = stbi_load(input_filepath, &width, &height, &bpp,  CHANNEL_NUM);

        // Check image load success
        if (image_result == NULL) {
            fprintf(stderr, "Error loading image in thread %d\n", thread_id);
            continue; // Skip this request if image loading failed
        }

        // Allocate memory for the result matrix (widths)
        uint8_t **result_matrix = (uint8_t **)malloc(sizeof(uint8_t*) * width);
        
        // Allocate memory to store the original image data in array format (widths)
        uint8_t** img_matrix = (uint8_t **)malloc(sizeof(uint8_t*) * width);
        
        // Allocate heights of the result and image matrices
        for(int i = 0; i < width; i++){
            result_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
            img_matrix[i] = (uint8_t *)malloc(sizeof(uint8_t) * height);
        }

        /*
        linear_to_image takes: 
            The image_result matrix from stbi_load
            An image matrix
            Width and height that were passed into stbi_load
        */

        // Fill in the image matrix with data from the original image
        linear_to_image(image_result, img_matrix, width, height);
        
        // You should be ready to call flip_left_to_right or flip_upside_down depends on the angle(Should just be 180 or 270)
        // both take image matrix from linear_to_image, and result_matrix to store data, and width and height.
        
        // Figure out which function we will call.
        if (request.rotation_angle == 180) { 
            flip_left_to_right(img_matrix, result_matrix, width, height);
        } else if (request.rotation_angle == 270) {
            flip_upside_down(img_matrix, result_matrix, width, height);
        }

        // Allocate memory for the 1D array of the result matrix
        uint8_t* img_array = (uint8_t *)malloc(sizeof(uint8_t) * width * height);
        
        // You should be ready to call flatten_mat function, using result_matrix
        // img_arry and width and height;

        flatten_mat(result_matrix, img_array, width, height);
        
        char output_filepath[1024];
        snprintf(output_filepath, sizeof(output_filepath), "%s/processed_%s", output_dir, request.filename);
        // You should be ready to call stbi_write_png using: New path to where you wanna save the file, Width height, img_array, width*CHANNEL_NUM
        stbi_write_png(output_filepath, width, height, CHANNEL_NUM, img_array, width*CHANNEL_NUM);
        
        // Free request filename from the processing thread
        free(request.filename);

        // Free all data that was allocated within the thread
        stbi_image_free(image_result);
        free(img_array);
        for (int i = 0; i < height; i++) {
            free(result_matrix[i]);
            free(img_matrix[i]);
        }
        free(result_matrix);
        free(img_matrix);

    }
}

/*
    Main:
        Get the data you need from the command line argument 
        Open the logfile
        Create the threads needed
        Join on the created threads
        Clean any data if needed. 


*/

int main(int argc, char* argv[]) {
    if(argc != 5)
    {
        fprintf(stderr, "Usage: File Path to image dirctory, File path to output dirctory, number of worker thread, and Rotation angle\n");
    }

    //Get arguments
    char* input_dir = argv[1];
    char* output_dir = argv[2];
    numWorkers = atoi(argv[3]);
    int rotationAngle = atoi(argv[4]);

    logFile = fopen(LOG_FILE_NAME, "w");

    if (logFile == NULL)
    {
        printf("Error creating the file.\n");
        return 1;
    }

    // request queue initialized
    initialize_request_queue(&global_request_queue, MAX_QUEUE_LEN);

    pthread_t threads[numWorkers]; // create numWorkers amount of threads
    pthread_t processing_thread;
    worker_args_t worker_args[numWorkers]; // create worker args for each worker thread
    processing_args_t proc_args;

    proc_args.input_dir = input_dir;
    proc_args.output_dir = output_dir;
    proc_args.rotation_angle = rotationAngle;
    proc_args.queue = &global_request_queue;

    if (pthread_create(&processing_thread, NULL, processing, &proc_args)) {
        fprintf(stderr, "Error creating thread \n");
        fclose(logFile);
        return 1;    
    }

    for (int i = 0; i < numWorkers; i++) { // set up args for each thread and create threads
        worker_args[i].thread_id = i;
        worker_args[i].request_count = 0;
        worker_args[i].input_dir = input_dir;
        worker_args[i].output_dir = output_dir;
        worker_args[i].queue = &global_request_queue;
        worker_args[i].log_file = logFile;
        int status = pthread_create(&threads[i], NULL, worker, (void *)&worker_args[i]); // create pthread

        if (status) { // check status
            fprintf(stderr, "Error creating thread %d\n", i);
            fclose(logFile);
            return status;
        }
    }

    pthread_join(processing_thread, NULL);

    for (int i = 0; i < numWorkers; i++) {
        pthread_join(threads[i], NULL); // join all pthreads
    }

    
    fclose(logFile);
    return 0;
}