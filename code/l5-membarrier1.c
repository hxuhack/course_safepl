#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> 
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>

//volatile int a = 1;
//atomic_int a = 1;
int a = 1;

#define barrier()__asm__ __volatile__("mfence": : :"memory")
#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))

void *t0 (void* in){
    //while (ACCESS_ONCE(a)) ; 
    while (a)  
        //barrier();
	;
}

void *t1 (void* in){
    a = 0;
}

int main(int argc, char** argv){
    pthread_t tid[2];
    assert(pthread_create(&tid[0], NULL, t0, NULL)==0);
    assert(pthread_create(&tid[1], NULL, t1, NULL)==0);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
}
