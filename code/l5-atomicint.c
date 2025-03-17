#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h> 
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

#define NUM 1000
int global_cnt = 0;
//atomic_int global_cnt=0;
//volatile int global_cnt=0;

void *mythread(void *from) {
    for (int i=0; i<NUM; i++) {
        //global_cnt++;
    	atomic_fetch_add(&global_cnt, 1);
    	//atomic_fetch_add_explicit(&global_cnt, 1, memory_order_acq_rel);
    }
}

int main(int argc, char** argv) {
    pthread_t tid[NUM];
    for (int i=0; i<NUM; i++){
        assert(pthread_create(&tid[i], NULL, mythread, NULL)==0);
    }
    for (int i=0; i<NUM; i++){
        pthread_join(tid[i], NULL);
    }
    assert(global_cnt==NUM*NUM);
    return 0;
}
