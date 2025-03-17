#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> 
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>

int a = 1;
int b = 1;

#define barrier()__asm__ __volatile__("": : :"memory")

void *t0 (void* in){
    a = 0;
    b = 0;
    //atomic_store_explicit(&b,0,memory_order_release);
    //atomic_store_explicit(&b,0,memory_order_relaxed);
    //atomic_store_explicit(&b,0,memory_order_seq_cst);
}

void *t1 (void* in){
    //while(atomic_load_explicit(&b,memory_order_seq_cst));
    //while(atomic_load_explicit(&b,memory_order_acquire));
    //while(atomic_load_explicit(&b,memory_order_relaxed));
    while(!b)
        assert(!a);
}

int main(int argc, char** argv)
{
    pthread_t tid[2];
    assert(pthread_create(&tid[0], NULL, t0, NULL)==0);
    assert(pthread_create(&tid[1], NULL, t1, NULL)==0);
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
}

