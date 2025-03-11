#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/resource.h>

#define LARGE_SIZE 1024L*1024L*1024L*256L


void main(void){
    char* p = malloc (LARGE_SIZE);
    if(p == 0) {
        printf("malloc failed\n");
    } else {  
        memset (p, 1, LARGE_SIZE);
    }
}

