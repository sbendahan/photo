#include "main.h"


/*************  Globals   *****************/
const char *semName = "/shmem_sem";
const key_t key = 5555;
char c;
int shmid;
shmem_t *shm_ptr; // pointeur debut de shared memoire
sem_t *sem_id;

/*************  Prototypes   *****************/
int init_shm();
int insert_sh_mem(char *photo, int fd);
void print_shm(char *start);
int server_send(char *data, char *ip_client);
int server_receive(int fd);
int server_sendphoto(void *photo);
int run_parent(int fd);

int run_parent(int fd)
{
    init_shm();
    server_receive(fd);
    return 0;
}

int server_receive(int fd)
{
    int sock, connected, bytes_recieved, true = 1;
    char send_data[SIZE_PHOTO], recv_data[SIZE_PHOTO];

    struct sockaddr_in server_addr, client_addr;
    int sin_size;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == -1)
    {
        perror("Setsockopt");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr.s_addr = INADDR_ANY; // ou gethostbyname()
    bzero(&(server_addr.sin_zero), 8);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("Unable to bind");
        exit(1);
    }

    if (listen(sock, 5) == -1) //<<<<<<<<<<<<<<<<<<<<<<<<<Blocking untill new connection is accepted <<<<<<<<<<<<<<<
    {
        perror("Listen");
        exit(1);
    }

    printf("\nTCPServer Waiting for client on port 5000");
    fflush(stdout);

    while (1)
    {

        sin_size = sizeof(struct sockaddr_in);

        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

        char *ip_client = inet_ntoa(client_addr.sin_addr);

        printf("\n I got a connection from (%s , %d)",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        bytes_recieved = recv(connected, recv_data, SIZE_PHOTO, 0);

        // ------ send back to the client
        for (int j = 0; j < SIZE_PHOTO; j++)
        {
            printf("0x%0X, ", recv_data[j]);
            printf("\n");
        }

        // insert_sh_mem(recv_data, shm_ptr->sem_id,shm_ptr->count_in);
        insert_sh_mem(recv_data, fd);
                        //  int data2=0;
            // if (write(fd, &data2, sizeof(data2)) < 0)
            // {
            //     return 1; // error
            // }
       //send to process child to treatement picture

        close(connected);
    }

    close(sock);
    return 0;
}

int server_send(char *photo, char *ip_client)
{

    int sock, bytes_recieved;
    char send_data[1024], recv_data[1024];
    struct hostent *host;
    struct sockaddr_in server_addr;

    // host = gethostbyname("127.0.0.1");
    host = gethostbyname(ip_client);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6000);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero), 8);

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1)
    {
        perror("Client APP not connected");
        exit(1);
    }

    sprintf(send_data, "%s", photo);
    printf(" send to client ; %s", send_data);
    send(sock, send_data, strlen(send_data), 0);
    printf(" send after ; %s", send_data);
    // close(connect);
    return 0;
}

int init_shm()
{
    // ----------------------------------------------------------------
    // ========= define/creat shared memory =======
    // ----------------------------------------------------------------

    sem_id = sem_open(semName, O_CREAT, 0600, 1);

    if (sem_id == SEM_FAILED)
    {
        perror("Parent  : [sem_open] Failed\n");
        exit(-1);
    }

    //  Create the segment of shared memory
    // SHMSZ  => bits change en page
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0)
    {
        perror("error shmget");
        exit(1);
    }

    // Now we attach the segment to our data space.
    // shm= pointer of the adress of my shmem
    if ((shm_ptr = shmat(shmid, NULL, 0)) == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }
    shm_ptr->count = 0;
    shm_ptr->ptr_head = shm_ptr->photo;
    shm_ptr->ptr_tail = shm_ptr->photo;

    // array_DB->photo=&shm;
    printf("server attached to memory START %p\n", shm_ptr);
}

int insert_sh_mem(char *photo, int fd)
{

    // ----------------------------------------------------------------
    // ========= insert photo in shared memory =======
    // ----------------------------------------------------------------
    //     if (sem_wait(sem_id) < 0)
    // {
    //     perror(" [sem_wait] Failed\n");
    //     exit(-2);
    // }
        memcpy((shm_ptr->ptr_head), photo, PHOTO_SIZE);
        if (write(fd, &(shm_ptr->ptr_head), sizeof(int)) < 0)
        {
            return 1; // error
        }

    if (shm_ptr->count < MAX_PHOTO)
    {
        shm_ptr->count++;
        printf("adr : %p\n", shm_ptr->ptr_head);
        shm_ptr->ptr_head++;
        if (shm_ptr->ptr_head > &(shm_ptr->photo[MAX_PHOTO - 1]))
        {
            shm_ptr->ptr_head = shm_ptr->photo;
        }
    }
    // if (sem_post(sem_id) < 0)
    // {
    //     perror(" [sem_post] Failed \n");
    //     exit(-6);
    // }

    return (0);
}

void close_shem(void)
{
    if (sem_close(sem_id) != 0)
    {
        perror("Parent  : [sem_close] Failed\n");
        exit(-3);
    }

    if (sem_unlink(semName) < 0)
    {
        perror("Parent  : [sem_unlink] Failed\n");
        exit(-4);
    }
    printf("server is ending\n");
}



