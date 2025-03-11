#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/resource.h>
#include <setjmp.h>
#include <signal.h>

#define SIGSTACK_SIZE 1024
#define LARGE_SIZE 1024L*1024L*1024L*256L
#define SMALL_SIZE 1024*20L

void exhaustheap(){
    for(long i=0; i < INT64_MAX; i++) {
	char* p = malloc (SMALL_SIZE);
        if(p == 0){
            printf("the %ldth malloc failed\n.", i);
	    break;
	}
	else {  
            printf("access the %ldth memory chunk.", i);
            memset (p, 0, sizeof (SMALL_SIZE));
            printf(", done\n", i);
	}
    }
}

void main(void){
    exhaustheap();
}

