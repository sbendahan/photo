#include "main.h"
#define SIZE_PHOTO 10

int server_send(char* data, char * ip_client);
int server_receive();




int main()
{
    server_receive();
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
        
        char * ip_client = inet_ntoa(client_addr.sin_addr);

        printf("\n I got a connection from (%s , %d)",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        bytes_recieved = recv(connected, recv_data, SIZE_PHOTO, 0);

        recv_data[bytes_recieved] = '\0';
        printf("SERVER: %s", recv_data);
        // ------ send back to the client 
        // sleep(1);
        server_send(recv_data,ip_client);

        close(connected);
    }

    close(sock);
    return 0;
}

int server_send(char* photo, char * ip_client)
{

    int sock, bytes_recieved;
    char send_data[1024], recv_data[1024];
    struct hostent *host;
    struct sockaddr_in server_addr;

    // host = gethostbyname("127.0.0.1");
    host=gethostbyname(ip_client);

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

    photo[9]=0;
    sprintf(send_data, "%s", photo);
    printf(" send to client ; %s", send_data);
    send(sock, send_data,strlen(send_data), 0);
    printf(" send after ; %s", send_data);

    return 0;
}