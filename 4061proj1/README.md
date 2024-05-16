###### Project group number
18

###### Group member names and x500s
Alexander Tatley, tatle010
Cole Schmidt, schm5346
Samuel Capece, capec016

###### The name of the CSELabs computer that you tested your code on
csel-kh1260-18.cselabs.umn.edu

###### Members’ individual contributions
Alex: created recursive merkle tree process start state, helped debug child_process.c
Cole: Leaf case, utils.c, debugging
Sam: Non-leaf case, hash concatenation, debugging 

###### Any changes you made to the Makefile or existing files that would affect grading
Additional modifications have been made to implement gdb debugging helper tools.
Debugging helper tools generated by chatgpt.

###### Any assumptions that you made that weren’t outlined in 7. Assumptions / Notes

We assumed that using pipes would be the only way to execute this task
(challenges surrounding communication with parent and child nodes), which Sam was
able to show wasn’t exactly true. The files were simply used as the storage for the development instead of using the pipes.

##### How you designed your program for creating the process tree 

##### (again, high-level pseudocode would be acceptable/preferred for this part). 

Argc check:
If argc !=5
	// Remind user of proper usage
	// return 1

int N = third argument

int ID = forth argument

If (ID is within range (N - 1) to (2 * N - 2)): // leaf case

    Initialize read_file_path
    
    Initialize current buffer
    
    Compute val as ID - (N - 1)
    
    Convert val to string, store in current
    
    Form read_file_path using current
    Open file at read_file_path (reading)
        If file opening fails:
            Report error, return 1
	    
    Initialize block_hash
    Compute hash of data in file, store in block_hash
    Close read_file_path file
    Get block_size as length of block_hash
    
    Initialize hash_path
    Form hash_path using ID
    Open hash_file file at hash_path (writing)
        If file opening fails:
            Report error, return 1
	    
    Write block_hash to file
    Close hash_file file
    
Else: // non-leaf case, concatenate hashes
    Compute left_child_ID as (2 * ID + 1)
    Compute right_child_ID as (2 * ID + 2)
    
    Fork to create left child process
        If in left child process:
            Convert left_child_ID to string
            Execute "./child_process" with left child arguments
    Fork to create right child process
        If in right child process:
            Convert right_child_ID to string
            Execute "./child_process" with right child arguments
	    
    Waitpid() for both left and right child processes to complete
    Initialize left_path, right_path
    Form left_path using left_child_ID
    Form right_path using right_child_ID
    Initialize left_hash, right_hash
    
    Open left_file file at left_path (reading)
    Read hash into left_hash
    Close left_file file
    
    Open right_file file at right_path (reading)
    Read hash into right_hash
    Close right_file file
    
    Initialize combined
    Compute hash of concatenated left_hash + right_hash, store in combined
    Initialize output_path
    Form output_path using ID
    
    Open output_path file at output_path (writing)
    Write combined hash to file
    Close output_path file



8. If your original design (intermediate submission) was different, explain how it changed

Pipes were not used, instead just used files directly for the hash calculations in program

9. Any code that you used AI helper tools to write with a clear justification and explanation of the

code (Please see below for the AI tools acceptable use policy)

Makefile was modified through Chatgpt assistance to enable easier gdb debugging.