#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>

int main(int argc, char** argv){

    char* ptr[1000000];


    //if you wwant true randomness uncomment these two lines, but it'll make it harder to debug
    //time_t t;
    //srand((unsigned) time(&t));


    //uncomment lines 32,40,43,51 if you want to see what you code is doing
    int operations = 0;
    printf("Enter number of operations between 1 and 5000000: ");
    scanf("%d", &operations);
    //code will seem slow but that's only because it takes printf forever to print out
    for( int i = 0; i < operations; ++i){
        //randomly pick a pointer
        int index = rand() % 100;
        int num = rand() % 2000 + 1;
        //if its null randomly allocate a random amount of memory
        if(ptr[index] == NULL){
            ptr[index] = malloc(num);
            *ptr[index] = num % 95 + 32;
            //fprintf(stdout, "Mallocing Pointer: %p  Value: %c Random num: %d Index: %d Operation num: %d\n", ptr[index], *ptr[index], num, index, i);
        }else{
            //else if the random is below 1000, randomly reallocate new memory
            if(num < 1000){
                int num = rand() % 2000 + 1;
                char test = *ptr[index];
                ptr[index] = realloc(ptr[index], num);
                //assert that the memory is still the same
                assert(*ptr[index] == test);
                //fprintf(stdout, "Reallocing Pointer: %p  Value: %c Random num: %d Index: %d Operation num: %d\n", ptr[index], *ptr[index], num, index, i);
            }else{
                free(ptr[index]);
                //fprintf(stdout, "Freeing Pointer: %p Index: %d\n", ptr[index], index);
                ptr[index] = NULL;
            }
        }
    }
    for(int i = 0; i < 100; ++i){
        if(ptr[i] != NULL){
            free(ptr[i]);
            //fprintf(stdout, "Freeing %p %d\n", ptr[i], i);
        }
    }

    printf("Your code runs successfully!!\n");
    
    return 0;
}