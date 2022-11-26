#include "main.h"

/*************  Globals   *****************/
const char *semName = "/shmem_sem";
const key_t key = 5555;
char c;
int shmid;
shmem_t *shm_ptr; // pointeur debut de shared memoire
sem_t *sem_id;

/*************  Prototypes   *****************/
int creat_shm();
int insert_sh_mem(char *photo, int fd);
void print_shm(char *start);
int server_send(char *data, char *ip_client);
int server_receive(int fd);
int server_sendphoto(void *photo);
int run_parent(int fd);
int remove_sh_mem(char *photo);
void config_serial(int fd_serial);
void uart1_send(char *photo);
// char* uart2_receive();

int run_parent(int fd)
{
    creat_shm();
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

    printf("\nTCPServer Waiting for client on port 5000\n");
    fflush(stdout);

    while (1)
    {

        sin_size = sizeof(struct sockaddr_in);

        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

        char *ip_client = inet_ntoa(client_addr.sin_addr);
        shm_ptr->client_ip=ip_client;

        // printf("\n I got a connection from (%s , %d)",
        //        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        bytes_recieved = recv(connected, recv_data, SIZE_PHOTO, 0);

        // ------ print --------------------------------
        // for (int j = 0; j < SIZE_PHOTO; j++)
        // {
        //     printf("0x%0X, ", recv_data[j]);
        // }
        // printf("\n");

        // insert and send to process child to treatement picture
        insert_sh_mem(recv_data, fd);

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
    printf(" send to client ; %s\n", send_data);
    send(sock, send_data, strlen(send_data), 0);
    // close(connect);
    return 0;
}

int creat_shm()
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

    sem_id = sem_open(semName, O_CREAT, 0600, 1);

    // ----------------------------------------------------------------
    // ========= insert photo in shared memory =======
    // ----------------------------------------------------------------

    memcpy((shm_ptr->ptr_head), photo, PHOTO_SIZE); // insert photo in memory
    // send photo to traitement
    if (write(fd, (void *)&(shm_ptr->ptr_head), sizeof(void *)) < 0)
    {
        return 1; // error
    }

    if (shm_ptr->count < MAX_PHOTO)
    {
        if (sem_wait(sem_id) < 0)
        {
            perror(" [sem_wait] Failed\n");
            exit(-2);
        }
        // printf("inser/  got semaphor\n ");
        shm_ptr->count++;
        if (sem_post(sem_id) < 0)
        {
            perror(" [sem_post] Failed \n");
            exit(-6);
        }
        // printf("insert/ release semaphor \n");
        // printf("insert to adr : %p\n", shm_ptr->ptr_head);
        shm_ptr->ptr_head++;
        if (shm_ptr->ptr_head > &(shm_ptr->photo[MAX_PHOTO - 1]))
        {
            shm_ptr->ptr_head = shm_ptr->photo;
        }
    }
    else{
        char*message ="full memory";
        server_send(message,shm_ptr->client_ip);
    }

    return (0);
}

int remove_sh_mem(char *photo)
{

    // ----------------------------------------------------------------
    // ========= remove photo from shared memory =======
    // ----------------------------------------------------------------

    sem_id = sem_open(semName, O_CREAT, 0600, 1);

    memcpy((shm_ptr->ptr_tail), photo, PHOTO_SIZE);

    if (shm_ptr->count != 0)
    {
        // printf("remove func\n");
        if (sem_wait(sem_id) < 0)
        {
            perror(" [sem_wait] Failed\n");
            exit(-2);
        }
        // printf("remove /got semaphore\n");
        shm_ptr->count--;
        if (sem_post(sem_id) < 0)
        {
            perror(" [sem_post] Failed \n");
            exit(-6);
        }
        // printf("remove /release semaphore\n");
        // printf("remove /adr tail: %p\n", shm_ptr->ptr_tail);
        shm_ptr->ptr_tail++;
        if (shm_ptr->ptr_tail > &(shm_ptr->photo[MAX_PHOTO - 1]))
        {
            shm_ptr->ptr_tail = shm_ptr->photo;
        }
    }
    else
    {
        printf("error to remove photo - the queue is empty\n");
    }

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

void config_serial(int fd_serial)
{
    struct termios options;

    tcgetattr(fd_serial, &options);                     // Get Current Config
    cfsetispeed(&options, B9600);                       // Set Baud Rate INPUT SPEED
    cfsetospeed(&options, B9600);                       // OUTPUT SPEED
    options.c_cflag = (options.c_cflag & ~CSIZE) | CS8; // CS8 => 8 bits (data)
    options.c_iflag = IGNBRK;
    options.c_lflag = 0;
    options.c_oflag = 0;
    options.c_cflag |= CLOCAL | CREAD;
    options.c_cc[VMIN] = 1;  // courant MAX VOLT
    options.c_cc[VTIME] = 5; // courant MIN VOLT
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_cflag &= ~(PARENB | PARODD); // nb de '1' est paire ou impaire
                                           //  parity => nb '1' paire

    /* Save The Configure */
    tcsetattr(fd_serial, TCSANOW, &options); // TCSANOW => execute now
    /* Flush the input (read) buffer */
    tcflush(fd_serial, TCIOFLUSH);
}

void uart1_send(char *photo)
{
    int ret, fd_serial1;
    struct termios options; // baud rate => debit
    int n;
    char tx_buff[255];
    /* Open Ports */
    fd_serial1 = open(SERIAL_DEVFILE_1, O_RDWR | O_NOCTTY | O_NDELAY); /* <--- serial port 1 */
    if (fd_serial1 == -1)
    {
        printf("ERROR Open Serial Port 1!");
        exit(-1);
    }
    config_serial(fd_serial1);
    // writing tx-buffer to serial port
    strcpy(tx_buff, photo);
    ret = write(fd_serial1, tx_buff, strlen(tx_buff));
    if (ret == -1)
    {
        perror("Error writing to device");
        exit(EXIT_FAILURE);
    }

    sleep(1); // wait for HW to write to device
    close(fd_serial1); // Close Port
}

// char* uart2_receive()
// {
//     int ret, fd_serial2;
//     struct termios options; // baud rate => debit
//     int n;
//     char rx_buff[255];
//     fd_serial2 = open(SERIAL_DEVFILE_2, O_RDWR | O_NOCTTY | O_NDELAY); /* <--- serial port 2 */
//     if (fd_serial2 == -1)
//     {
//         printf("ERROR Open Serial Port 2!");
//         exit(-1);
//     }
//     // Serial Configuration
//     config_serial(fd_serial2);
//     // reading serial port into rx-buffer
//     ret = read(fd_serial2, rx_buff, strlen(rx_buff));
//     if (ret == -1)
//     {
//         perror("Error writing to device");
//         exit(EXIT_FAILURE);
//     }
//     close(fd_serial2); // Close Port
//     return rx_buff;
// }