#include "main.h"

// #define SIZE_PHOTO 1000
#define SIZE_PHOTO 10

extern pthread_mutex_t sync_rcv;

// process 2
// ouvertur connexion port 6000
// while nb photo
// reception reponse de la port 6000
// affichage de la rep
// nb--
// end while
// fermetur porte 6000

void * client_receive(void * a)
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
    server_addr.sin_port = htons(6000);
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
    printf("init_receive completed\n");
    pthread_mutex_unlock(&sync_rcv);


    while (1)
    {

        sin_size = sizeof(struct sockaddr_in);

        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        
        // 
        char * ip_client = inet_ntoa(client_addr.sin_addr);

        printf("\n I got a connection from (%s , %d)\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        bytes_recieved = recv(connected, recv_data, SIZE_PHOTO, 0);

        // recv_data[bytes_recieved-1] = '\0';
        recv_data[bytes_recieved] = '\0';

        printf("SERVER: %s\n", recv_data);
        // ------ send back to the client 
        close(connected);
    }

    close(sock);
    return 0;
}
