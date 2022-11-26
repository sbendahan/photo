#ifndef MAIN_H
#define MAIN_H

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
#include <termios.h> //use for uart
#include <stdarg.h>

#include <sys/stat.h> /* For mode constants */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>    /* For O_* constants */
#include <sys/stat.h> /* For mode constants */

#include <semaphore.h>
#include <sys/wait.h>

#include <sys/ipc.h>
#include <sys/shm.h>

//************ DEBUG ***************
extern FILE *logptr;
// extern process_t proc[4];

extern void _debug (const char *fmt, ...);

#define DEBUG(X)     logptr = fopen("debug.txt","a");fprintf(logptr,"pp:%d p:%d %s:%d ",getppid(),getpid(),__FILE__, __LINE__); _debug X;fprintf(logptr, "\n");fclose(logptr);
/*************  definitions     *****************/
#define SIZE_PHOTO 10
#define PHOTO_SIZE 20
#define MAX_PHOTO 5

#define SERIAL_DEVFILE_1 "/dev/ttyS1"
#define SERIAL_DEVFILE_2 "/dev/ttyS2"

typedef struct photo_t
{
    char elemt[PHOTO_SIZE];
} photo_t;
typedef struct shmem_t  // shared memory struct
{
    photo_t photo[MAX_PHOTO];
    int count;         // counter of photo in sh mem
    photo_t *ptr_head; // ptr to add in shared mem
    photo_t *ptr_tail; // ptr to read & remove to shared mem
    char* client_ip;
} shmem_t;

#define SHMSZ sizeof(shmem_t)

#endif