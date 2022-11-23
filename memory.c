#include <string.h>
#define PHOTO_SIZE 1000
#define NUM_PHOTOS 50

//test
// struct photo_t contains photo and pointer to next
typedef struct photo_t
{
    char photo[PHOTO_SIZE];
    void *next;
} PHOTO_T;

// shared memory structure
typedef struct shared_memory_t
{
    PHOTO_T *start_list;
    PHOTO_T *end_list;
    PHOTO_T *next_mem_alloc;
    PHOTO_T *next_mem_free;
    PHOTO_T *StartBuffer;
    PHOTO_T *EndBuffer;
    int photo_cnt;
    PHOTO_T photo[NUM_PHOTOS];
} SHARED_MEMORY_T;

// assign shared memory
SHARED_MEMORY_T memory;

// photo: IN pointer to photo;
// p: IN pointer to shared memory
// Add photo to end of list
PHOTO_T *AddPhoto(SHARED_MEMORY_T *p, char *photo);

// p: IN pointer to shared memory
// remove first entered photo in the list
void RemovePhoto(SHARED_MEMORY_T *p);

// p: IN pointer to shared memory
// give pointer to next available photo in circular photo buffer
PHOTO_T *mymalloc(SHARED_MEMORY_T *p);

// p: IN pointer to shared memory
// remove first photo in circular photo buffer
void myfree(SHARED_MEMORY_T * p);

void InitSharedMemory(SHARED_MEMORY_T *p)
{
    p->photo_cnt = 0;
    p->next_mem_alloc = p->photo;
    p->next_mem_free = p->photo;
    p->StartBuffer = p->photo;
    p->EndBuffer = p->photo + NUM_PHOTOS; 
}

PHOTO_T *AddPhoto(SHARED_MEMORY_T *p, char *photo)
{
    // Add photo to end of list
    PHOTO_T *ptr = mymalloc(p);
    if (ptr != 0)
    {
        memcpy(ptr, photo, PHOTO_SIZE);
        if (p->start_list == 0)
        {
            p->start_list = ptr;
        }
        else
        {
            p->end_list->next = ptr;
        }
        ptr->next = 0;
        p->end_list = ptr;
        return ptr;
    }
        return NULL;   
}

    void RemovePhoto(SHARED_MEMORY_T * p)
    {
        // remove photo from begining of list (FIFO)
        if (p->start_list != 0)
        {
            myfree(p);
            p->start_list = p->start_list->next;
        }
    }

    PHOTO_T *mymalloc(SHARED_MEMORY_T * p)
    {
        PHOTO_T* ptr= p->next_mem_alloc;
        if (p->photo_cnt < NUM_PHOTOS)
        {   
             (p->photo_cnt)++;
             (p->next_mem_alloc)++;
             //check end of circular Buffer
             if (p->next_mem_alloc == p->EndBuffer){
                p->next_mem_alloc=p->StartBuffer;
             } 
             return ptr;
        }
        return NULL; //NULL pointer
    }

    void myfree(SHARED_MEMORY_T * p)
    {
        PHOTO_T* ptr= p->next_mem_alloc;
        if (p->photo_cnt > 0)
        {   
             (p->photo_cnt)--;
             (p->next_mem_free)++;
             //check end of circular Buffer
             if (p->next_mem_free == p->EndBuffer){
                p->next_mem_free=p->StartBuffer;
             } 
        }
    }