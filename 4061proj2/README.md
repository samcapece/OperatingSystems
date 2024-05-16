# project2
README
Project Group Number: 18

Group Members and x500s:
Alexander Tatley, tatle010
Cole Schmidt, schm5346
Samuel Capece, capec016

CSELabs Computer Tested On:
csel-kh1260-18.cselabs.umn.edu

Changes Made to the Makefile or Existing Files:
None.

Division of Work:

Alexander:
Responsible for the implementation of the root process.
Setting up the root process logic.
Handling the pipe and fork for the first non-leaf process.
Constructing the long string that contains all file names and file hashes.
Locating the duplicates and parsing the string to map file names and hashes.

Cole:
Responsible for the leaf process.
Writing the obtained hash to the pipe.
Handling the creation of symbolic links and deleting duplicate files.

Sam
Responsible for the non-leaf process.
Setting up the logic for iterating through the directory.
Handling the decision-making of whether the current entry is a directory or a file.
Setting up the pipe, fork, and exec for subsequent child processes.

Plan for Project:
    Root Process:
        Take the target directory as input.
        Set up a pipe for communication.
        Fork to create a child process.
        If it’s a directory, use exec() to run the non-leaf process program.
        If file, exec() the leaf process.
        Parents use the read end of the pipe to gather file hashes after children finish.
        
        Non-Leaf Process:
            Accepts path to target directory and write end of pipe.
            For every entity in the directory:
            Set up the pipe.
            Fork.
            If it's a directory, use exec() to run the non-leaf process.
            If it's a file, use exec() to run the leaf process.
        
        Leaf Process:
            Compute the file's hash.
            Write to the write end of the pipe in the format filename|hash|.

    Manage Duplicates:
        After gathering all filenames and hashes, look for duplicates.
        For each duplicate, delete the file with a higher number and create a symbolic link pointing to the file with the lower number.
        
    Error Handling and Memory Management:
        Ensure all functions and their calls are checked for errors.
        Initialize buffers using memset().
        Free any dynamically allocated memory.

Any assumptions that you made that weren’t outlined in 7. Assumptions / Notes
    No. We didn't make any new assumptions that weren't outlined in 7.-
How you designed your program for creating the process tree (again, high-level pseudocode
would be acceptable/preferred for this part)
    