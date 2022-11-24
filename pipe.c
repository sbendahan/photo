

/*************  includes     *****************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> //used for exit
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <error.h>

/*************  main() function ****************/
// we have 4 process and 3 pipes 

char* traitement_pipe(char*photo)
{

    int fds[2][2];
    int i;
    pid_t pid1, pid2, pid3;
    
    // Create a pipe. File descriptors for the two ends of the pipe are placed in fds
    for (i = 0; i < 2; i++){
        if (pipe(fds[i]) < 0){
            perror("pipe failed");
        }
    }

    /************ Fork process 1 *****************/ // DEV

    pid1 = fork();
    if (pid1 < 0){
        printf("error pid1");
        exit(1);
    }
    if (pid1 == (pid_t)0){
        //  Close our copy of the write end of the file descriptor.
        close(fds[0][1]); close(fds[1][0]); 
        // read data
        int data;
        if (read(fds[0][0], &data, sizeof(int)) < 0){
            exit(1); // error
        }
        data += 5; // do operation on the photo;
        printf("child1: %s\n",data);
        if (write(fds[1][1], &data, sizeof(int)) < 0){
            exit(1); // error
        }
        close(fds[0][0]); close(fds[1][1]);
    }

    /************ Fork process 2 *****************/ // ID

    pid2 = fork();
    if (pid2 < 0){
        printf("error pid1");
        exit(1);
    }
    if (pid2 == (pid_t)0){
        close(fds[0][0]); close(fds[0][1]);close(fds[1][1]);
        // read data
        int data;
        if (read(fds[1][0], &data, sizeof(int)) < 0){
            return 1; // error
            }
         data += 2 *5; // do operation on the photo;
        printf("child2: %s\n",data);
        close(fds[1][0]);
    }

    /************ PARENT process *****************/  
   

    close(fds[0][0]); close(fds[1][0]); close(fds[1][1]); 
    int * data = photo;
    if (write(fds[0][1], &data, sizeof(int)) < 0){
        return 1; // error
    }
    close(fds[0][1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    return 0;
}
