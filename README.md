**********************************************************************************************************
                          
				
				              C S E   -   5 0 6   S B U N I X



		           P R O J E C T    F E A T U R E S  :  Preemptive OS with  File System

                                     P R O J E C T    P O I N T S      :  7 0

				

**********************************************************************************************************


                                          I M P O R T A N T     P O I N T S	


**********************************************************************************************************


FILE SYSTEM DETAILS:

Use the disk named "sbunix_hdd.img" in cse506 folder. Disk Image size is 20 MB

If new disk is to created, here are the following steps:

    1. Run the qemu command to create a 20 MB disk named "sbunix_hdd.img" : qemu-img create sbunix_hdd.img 20M 

    2. Insert the superblock to that image using the following command :  
                  dd if=supeblock of=sbunix_hdd.imgseek=1 conv=notrunc 
    (superblock file is provided in the home directory, 
    which is being generated using intialize_superblock.c)

    3. Now the sbunix_hdd.img is ready to be used for the operating system. 


SHUTDOWN DETAILS:

Type shutdown on shell.
                            
SHELL COMMANDS :

Supported commands are : 
cd, cd .., ls, cat, shutdown, echo, sleep, ps, sh, man, pwd, vi.


**********************************************************************************************************



                                 S Y S T E M      C A L L S      D E S C R I P T I O N 

**********************************************************************************************************
    
    Name         :  READ()
    Syntax       :  read(file *file, uint64_t addr, char *buf)
    Description  :  This system call attempts to read  the file at the address addr into the buffer buf
    Dependencies :  Always open a file using OPEN system call before reading and close the file after 
		    completion. Returns -1 for files with WRONLY flag set.
    Return       :  In the case of success, it returns the length of bytes read into the buffer while in
                    the case of error, it returns a value of -1



    Name         :  WRITE() 
    Syntax       :  int write(file *fd, char *buf, int size)
    Description  :  This system call attempts to write up to size (number of bytes) into file descriptor 
                    fd from the buffer buf.
    Dependencies :  Always open a file using OPEN system call before writing and close the file after 
		    completion. 
                    Returns -1 for files opened with O_RDONLY flag set. 
                    Cannot do write on files in tarfs.
    Return       :  In the case of success, it returns the length of bytes written into the 
                    buffer while in the case of error returns the value of -1

    
    Name         :  MALLOC()
    Syntax       :  void *malloc(uint64_t size)
    Description  :  This system call increases the size of vma by size
    return       :  It returns the virtual address in heap

    
    Name         :  FORK()
    Syntax       :  pid_t fork(void)
    Description  :  This system call creates a child process. As a return value, it returns pid = 0 for 
                    child and non zero for parent.
    return       :  It returns the pid for parent process or zero to child 

    
    Name         :  EXECVPE()
    Syntax       :  execvpe(char *file, char *argv[], char *envp[])
    Description  :  This system call executes a file along with any necessary arguments argv[] in the 
                    environment envp[].
    return       :  In case of failure, it returns a value of -1 while in the case of success it does not 
                    return

    
    Name         :  WAIT()
    Syntax       :  wait(uint64_t status)
    Description  :  This system call is used for the parent process to wait for a child process to exit.
    Dependencies :  It returns a value of -1 if parent process has no children processes.

    
   
    
    Name         :  EXIT()
    Syntax       :  exit(int status)
    Description  :  This system call immediately terminates the calling process.


    
    Name         :  GETPID()
    Syntax       :  getpid()
    Description  :  This system call returns the process id of currently running process.


    
    Name         :  GETPPID()
    Syntax       :  getppid()
    Description  :  This system call returns the parent's process id of currently running process.


    
    Name         :  PS()
    Syntax       :  ps()
    Description  :  This system call gives the list of all currently running process.
    


    Name         :  OPENDIR()
    Syntax       :  opendir(uint64_t* entry, uint64_t* directory)
    Description  :  This system call returns a directory struct with list of all inode/files in it.
    Return       :  on error returns directory struct with values set to null.   
   

    Name         :  READDIR()
    Syntax       :  readdir(DIR* node)
    Description  :  This system call lists contents in directory and iteratively call readdir. 
    Dependencies :  Read directory is followed after open directory system call.
    Return       :  returns the pointer to dirent structure .
                    Returns NULL if reached end of directory or on occurence of an error. 



 
    
    Name         :  OPEN()
    Syntax       :  open(char* dir_path, uint64_t flags)
    Description  :  This system call opens a files and returns a file descriptor. Open files from tarfs and                    disk depending on dir_path.
    Dependencies :  flags can be O_CREAT, O_RDONLY, O_WRONLY, O_APPEND, O_TRUNC, O_RDWR
    Return       :  returns file descriptor number. open on a directory returns -1 as file descriptor.


    
   
    Name         :  SLEEP()
    Syntax       :  sleep(int msec)
    Description  :  This system call changes the state of current task to sleep for msec milliseconds.


    
    Name         :  CLEAR()
    Syntax       :  clear()
    Description  :  This system call clears the screen.
    


    Name         :  SEEK()
    Syntax       :  seek(uint64_t fd_type, int offset, int whence) 
    Description  :  This system call moves the current file pointer by the offset number of bytes in the 
                    file.
    Dependencies :  whence can have the values of SEEK_SET, SEEK_CUR, SEEK_END. This determines the offset                     from start, current pointer or end of file.



    Name         :  SHUTDOWN()
    Syntax       :  shutdown() 
    Description  :  This system call exits all the processes. 
