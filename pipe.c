#include "main.h"

/*************  main() function ****************/
extern const char *semName;
extern const key_t key;
shmem_t *shm_ptr; // pointeur debut de shared memoire
int init_shmem();

int run_parent(int fd);

// we have 4 process and 3 pipes
// TODO : do the prog in loop

int main()
{

    char ch;

    int fds[3][2];
    int i;
    pid_t pid1, pid2, pid3,pid4;

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
        close(fds[0][1]); // fermetur write du pere
        close(fds[1][0]); // fermeture lecture du fils
        close(fds[2][0]); // fermeture de write fork2
        close(fds[2][1]); // fermeture lecture fork2
        while (1)
        {
            // read data
            void *ptr;
            if (read(fds[0][0], &ptr, sizeof(ptr)) < 0)
            {
                exit(1); // error
            }
            // data += 5; // do operation on the photo;
            printf("child1: %p\n", ptr);
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
            // data += 5; // do operation on the photo;
            printf("child2: %p\n", ptr);
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
            void *ptr; // do operation on the photo;
            if (read(fds[2][0], &ptr, sizeof(int)) < 0)
            {
                return 1; // error
            }
            printf("the data is: %p\n", ptr);
        }
        close(fds[2][0]);
    }

    /************ PARENT process *****************/ // INC + send to UART1

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