#include "main.h"
/*************  definitions     *****************/
#define SIZE_PHOTO 10
#define PHOTO_SIZE 1000
#define MAX_PHOTO 50
#define SHMSZ 50000
typedef struct photo_t
{
    char photo[PHOTO_SIZE];
} photo_t;

typedef struct shmem_t
{
    photo_t p[PHOTO_SIZE];
    const char *semName;
    int count_in;
    int count_out;
} shmem_t;

photo_t array_DB[MAX_PHOTO];

/*************  Globals   *****************/
const char *semName = "shmem";
char c;
int shmid;
key_t key;
char *shm;
photo_t array_DB[MAX_PHOTO];
shmem_t my_mem;
#define SHMSZ sizeof(my_mem)

/*************  Prototypes   *****************/
int intit_shm();
int insert_sh_mem(photo_t *photo, sem_t *sem_id, int index);
void print_shm(char *start);
int server_send(char *data, char *ip_client);
int server_receive();

int main()
{
    intit_shm();
    pid_t pid1 = fork();
    if (pid1 < 0)
    {
        printf("error pid1");
        exit(1);
    }
    if (pid1 == (pid_t)0)
    {
        char *p = "hello";
        traitement_pipe(p);
        sleep(1);
    }
    else
    {
        server_receive();
    }
    return 0;
}

int server_receive()
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

        recv_data[bytes_recieved - 1] = '\0';
        printf("SERVER: %s", recv_data);
        // ------ send back to the client
        sleep(1);
        server_send(recv_data, ip_client);

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

int intit_shm()
{
    // ----------------------------------------------------------------
    // ========= define/creat shared memory =======
    // ----------------------------------------------------------------

    printf("main started\n");
    sem_t *sem_id = sem_open(semName, O_CREAT, 0600, 1);
    if (sem_id == SEM_FAILED)
    {
        perror("Parent  : [sem_open] Failed\n");
        exit(-1);
    }

    // name our shared memory segment "5678".
    key = 5555;

    //  Create the segment of shared memory
    // SHMSZ  => bits change en page
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0)
    {
        perror("error shmget");
        exit(1);
    }

    // Now we attach the segment to our data space.
    // shm= pointer of the adress of my shmem
    if ((shm = shmat(shmid, NULL, 0)) == (char *)-1)
    {
        perror("shmat");
        exit(1);
    }

    // array_DB->photo=&shm;
    printf("server attached to memory START %p\n", shm);
    //======================================
    photo_t photo; //= "your photo";
    strcpy(photo.photo, "data");
    photo_t *array_DB = (photo_t *)shm;

    printf("adress 1 %p\n", array_DB);

    // memcpy
    int count = 0;
    while (1)
    {
        while (count < 50)
        {
            insert_sh_mem(&photo, sem_id, count);
            count++;
        }
    }
    print_shm(shm);
    //======================================

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

int insert_sh_mem(photo_t *photo, sem_t *sem_id, int index)
{

    // ----------------------------------------------------------------
    // ========= insert photo in shared memory =======
    // ----------------------------------------------------------------

    if (sem_wait(sem_id) < 0)
    {
        perror(" [sem_wait] Failed\n");
        exit(-2);
    }
    strcpy(array_DB[index].photo, photo);
    printf("adr : %p\n", &array_DB[index]);
    if (sem_post(sem_id) < 0)
    {
        perror(" [sem_post] Failed \n");
        exit(-6);
    }

    return (0);
}
