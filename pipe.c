#include "main.h"

/************* global ****************/
extern const char *semName;
extern const key_t key;
extern shmem_t *shm_ptr; // pointeur debut de shared memoire
enum
{
    CAT = 1,
    MOUSE,
    DOG
};
/*************  prototypes ****************/
int init_shmem();
int remove_sh_mem(char *photo);
void uart1_send(char *photo);
int server_send(char *data, char *ip_client);
void send_respons(char*resp);
void uart2_receive();
int run_parent(int fd);

/************* SERVER main() function ****************/

int main()
{

    char ch;
    int fds[3][2];
    int i;
    char *tx;
    pid_t pid1, pid2, pid3, pid4;

    //  ************** open PIPE **********************
    // Create a pipe. File descriptors for the two ends of the pipe are placed in fds
    for (i = 0; i < 3; i++)
    {
        if (pipe(fds[i]) < 0)
        {
            perror("pipe failed");
        }
    }

    /************ Fork process 1 *****************/ // fft

    pid1 = fork();
    if (pid1 < 0)
    {
        printf("error pid1");
        exit(1);
    }
    if (pid1 == (pid_t)0)
    {
        // child 1
        init_shmem();

        //  Close our copy of the write end of the file descriptor.
        close(fds[0][1]); // close write parent
        close(fds[1][0]); // close read child
        close(fds[2][0]); // close  write fork2
        close(fds[2][1]); // close read fork2
        while (1)
        {
            // read data
            char *ptr;
            if (read(fds[0][0], &ptr, sizeof(ptr)) < 0)
            {
                exit(1); // error
            }
            // TODO // do operation on the photo;
            printf("FFT: %p\n", ptr);

            if (write(fds[1][1], &ptr, sizeof(ptr)) < 0)
            {
                exit(1); // error
            }
            sleep(1);
        }

        close(fds[0][0]);
        close(fds[1][1]);
        exit(0);
    }

    /************ Fork process 2 *****************/ // dev

    pid2 = fork();
    if (pid2 < 0)
    {
        printf("error pid1");
        exit(1);
    }
    if (pid2 == (pid_t)0)
    {

        init_shmem();
        close(fds[0][0]);
        close(fds[0][1]);
        close(fds[1][1]);
        close(fds[2][0]);
        while (1)
        { // read data
            void *ptr;
            if (read(fds[1][0], &ptr, sizeof(ptr)) < 0)
            {
                exit(1); // error
            }
            //TODO // do operation on the photo;
            printf("CONV: %p\n", ptr);
            if (write(fds[2][1], &ptr, sizeof(ptr)) < 0)
            {
                exit(1); // error
            }
            sleep(1);
        }

        close(fds[1][0]);
        close(fds[2][1]);
        exit(0);
    }

    /************ Fork process 3 *****************/ // id >>>>> uart1

    pid3 = fork();
    if (pid3 < 0)
    {
        printf("error pid1");
        exit(1);
    }
    if (pid3 == (pid_t)0)
    {

        init_shmem();
        close(fds[0][0]);
        close(fds[0][1]);
        close(fds[1][0]);
        close(fds[1][1]);
        close(fds[2][1]);
        while (1)
        {
            void *ptr; 
            int x = random() % 3+1;
            char*resp;
            printf("x= %d\n",x);
            if (read(fds[2][0], &ptr, sizeof(ptr)) < 0)
            {
                return 1; // error
            }
            printf("ID: %p\n", ptr);
        
            // sleep(5);
            if (x == 1)
            {
                strcpy(resp, "CAT");
                printf( "%s\n",resp);
                uart1_send(resp);
            }
            else if (x == 2)
            {
                strcpy(resp, "MOUSE");
                printf( "%s\n",resp);
                uart1_send(resp);
            }
            else if (x == 3)
            {
                strcpy(resp, "DOG");
                printf( "%s\n",resp);
                uart1_send(resp);
            }
            remove_sh_mem(ptr);
        }
        close(fds[2][0]);
    }

    /************ fork 4 :TCP process *****************/ // connection TCP

    pid4 = fork();
    if (pid4 < 0)
    {
        printf("error pid1");
        exit(1);
    }
    if (pid4 == (pid_t)0)
    {
        close(fds[0][0]);
        close(fds[1][0]);
        close(fds[1][1]);
        close(fds[2][1]);
        close(fds[2][0]);

        run_parent(fds[0][1]);

        close(fds[0][1]);
        exit(0);
    }

    /************ parent process *****************/ // quit app

    uart2_receive();
    close(fds[0][0]);
    close(fds[0][1]);
    close(fds[1][0]);
    close(fds[1][1]);
    close(fds[2][0]);
    close(fds[2][1]);
    printf("Enter q to stop application\n");
    while ((ch = getchar()) != 'q')
    {
        sleep(1);
    }
    kill(pid1, SIGKILL);
    kill(pid2, SIGKILL);
    kill(pid3, SIGKILL);
    kill(pid4, SIGKILL);
    // waitpid(pid4, NULL, 0);
    // waitpid(pid1, NULL, 0);
    // waitpid(pid2, NULL, 0);
    // waitpid(pid3, NULL, 0);
    return 0;
}

int init_shmem()
{
    int shmid;
    int retval;

    printf("main started\n");
    sem_t *sem_id = sem_open(semName, O_CREAT, 0600, 1);

    if (sem_id == SEM_FAILED)
    {
        perror("Child   : [sem_open] Failed\n");
        exit(-5);
    }
    if ((shmid = shmget(key, SHMSZ, 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }

    if ((shm_ptr = shmat(shmid, NULL, 0)) == (shmem_t *)-1)
    {
        perror("shmat");
        exit(1);
    }
    printf("client attached to memory %p\n", shm_ptr);
}