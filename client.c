#include "main.h"

FILE *logptr;
void photo(void *p);

void * client_receive(void * a);

void _debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(logptr, fmt, args);
    va_end(args);
}
pthread_mutex_t sync_rcv;

// main function

int main(int argc, char **argv)
{
    int i;
    int count;
    logptr = fopen("debug.txt", "w");
    fclose(logptr);
    DEBUG(("App Started Arg count:%d", argc));
    for (i = 0; i < argc; i++)
    {
        DEBUG(("Arg %d: %s", i, argv[i]));
    }
    const int NUM_PHOTO = 10; //>>>>>>>>>>num photo that we send to the server 
    int num_photo = NUM_PHOTO;

    DEBUG(("NUM PHOTOS: %d", num_photo));

    // process : receive 
    pthread_t pthread2;
    // init_receive();
    pthread_mutex_lock(&sync_rcv);
    pthread_create(&pthread2, NULL,(void*)client_receive, NULL);
    pthread_mutex_lock(&sync_rcv);

    printf ("Ready To receive\n");

    // process : send 
    pthread_t pthread;
    pthread_create(&pthread, NULL,(void*)photo, &num_photo);
    

    char ch;
    printf("Enter q to stop application\n");
    while ((ch = getchar()) != 'q')
    {
        sleep(1);
    }
    
    // send to the server / to close the socket ???
}



