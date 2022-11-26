
#include "main.h"

#define SIZE_PHOTO 10

int sock, bytes_recieved;
char send_data[1024], recv_data[1024];
struct hostent *host;
struct sockaddr_in server_addr;

void init_send()
{

    host = gethostbyname("127.0.0.1");

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero), 8);

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1)
    {
        perror("RUN SERVER APP before Running Client");
        exit(1);
    }
}

int client_send(char *pic)
{
    init_send();
    // sprintf(send_data, "%s", pic);
    send(sock, pic, SIZE_PHOTO, 0);
    return 0;
}

void photo(void *p)
{
    int num_photo = *(int *)p;
    DEBUG(("creat photo process started : num = %d", num_photo))
    // ouvertur de la connexion
    // while nb de photo
    while (num_photo)
    {
        // creat photo malloc (SIZE_PHOTO)
        char *photo = malloc(SIZE_PHOTO);
        sprintf(photo,"photo %d\n", num_photo);
        // send photo port 5000
        client_send(photo);
        DEBUG(("AFTER SEND"));
        // nb --
        num_photo--;
        free(photo);
        // sleep(2);
    }
    // end while

}
