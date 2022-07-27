#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define list_size 9
#define page_size 4096
#define block_max 1024


void __attribute__((constructor)) build_map();
void* freelist_get(void* head, int map_page_size);
void*new_map(int size);

void * map_list[list_size];
int fd;
double log_2;


void build_map(){
    fd = open ("/dev/zero", O_RDWR ) ;
    for(int i = 0; i < list_size; ++i){
        map_list[i] = NULL;
    }
    log_2 = log(2);
}

void * malloc(size_t size){

    if(size == 0) return NULL;
    unsigned map_page_size = size;
    if(size <= block_max){
        if (size < 8) map_page_size = 7;
        map_page_size = (int)ceil(log(size)/log_2);

        //map_page_size--;
        //map_page_size |= map_page_size >> 1;
        //map_page_size |= map_page_size >> 2;
        //map_page_size |= map_page_size >> 4;
        //map_page_size |= map_page_size >> 8;
        //map_page_size |= map_page_size >> 16;
        //map_page_size++;
        
        int i = log(map_page_size)/log_2 - 3;
        //if empty make it not empty
        if(map_list[i] == NULL){
            map_list[i] = new_map(map_page_size);
        }
        //one short and long page start. short for the pointer arithmetic
        short* short_page_start = map_list[i];
        long* long_page_start = map_list[i];
        long* next_page = (long*)*long_page_start;
        short* free_list = short_page_start + 5;
        short offset = *(free_list);

        //if offset is 0 then our map is full
        if(offset == 0){ 
            long_page_start = freelist_get(map_list[i], map_page_size);
            short_page_start = (short*)long_page_start;
            next_page = (long*)*long_page_start;
            free_list = short_page_start + 5;
            offset = *(free_list);
        }
        //make the return pointer and update the free list
        short * return_ptr = (short*)((long) short_page_start | (long)offset);
        *free_list = *(return_ptr + map_page_size/2);
        return return_ptr;
        
    }else
    return NULL;
}
void* freelist_get(void* head, int map_page_size){
    //initialize header info
    short* start_of_page = head;
    long* long_page_start = head;
    long* next_page = head;
    next_page = (long*)*next_page;
    short* free_list = (start_of_page + 5);

    //if the free_list is not null return the page
    if(*free_list != 0){
        return head;
    }else{
        //if free_list is null and next_page is null, make a new page
        if(next_page == NULL){
            long* newptr = new_map(map_page_size);
            *long_page_start = (long)newptr;
            return newptr;
        }
    }
    //recursive
    return freelist_get(next_page, map_page_size);
}
void free(void * ptr){
    if(ptr == NULL) return;

    //get beginning
    long temp = (long)ptr & ~0xfff;
    short* short_page_start = (short*)temp;
    long* long_page_start = ptr;
    long_page_start--;
    int map_page_size;

    //if first bit is a 1 (big)
    if(*long_page_start < 0){
        //unmap the big
        map_page_size = *long_page_start & 0x7fffffffffffffff;
        munmap(ptr, map_page_size);
        return;
    }else{
        //get the size of the page
        map_page_size = *(short_page_start + 4);
        //get the free_list of the page
        short* free_list = short_page_start + 5;
        //get offset
        short offset = *free_list;
        //get pointer
        short* freed_ptr = (short*)ptr;
        //set the pointer to its personal pointer in the map
        freed_ptr += map_page_size/2;
        //set it equal to the offset which is what the free_list is pointing to
        *freed_ptr = offset;
        //set the free_list = to the pointer
        *free_list = (short)((short)ptr & 0xfff);
    }  
}

void * calloc(size_t num, size_t size){
    void * page = malloc(num * size);
    memset(page, 0, num * size);
    return page;
}

void * realloc(void * ptr, size_t size){
    if(ptr == NULL) return malloc(size);

    long temp = (long)ptr & ~0xfff;
    long* long_page_start = ptr;
    long_page_start--;
    int old_length = 0;


    if(*long_page_start < 0){
        old_length = (*long_page_start & 0x7fffffffffffffff);
    }else{
        short* small_page = (short*)temp;
        old_length = *(small_page + 4);
    }
    
    void* newptr = malloc(size);
    if(size < old_length)
        memcpy(newptr, ptr, size);
    else    
        memcpy(newptr, ptr, old_length);
    free(ptr);
    return newptr;

}

void* new_map(int map_page_size){
        short* temp;
        long* ptr;
        void * map = mmap ( NULL , page_size , PROT_READ | PROT_WRITE , MAP_PRIVATE | MAP_ANONYMOUS , -1 , 0);


        //0x...000
        temp = map;
        ptr = map;
        *ptr = 0;

        //0x...008
        temp+=4;
        *temp = map_page_size;

        //0x...00a = 0x...00c
        temp++;
        *temp = (short)(temp + 1) & 0x0fff;
        temp++;

        while( page_size - ((long) (temp + map_page_size/2 + 1) & 0xfff) > map_page_size + 2){
            temp += map_page_size/2;
            *temp = (short)(temp + 1) & 0x0fff;
            temp++;
        }
        temp += map_page_size/2;
        *temp = 0;
        temp = map;

        return map;
}
