#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/resource.h>
#include <setjmp.h>
#include <signal.h>

#define SIGSTACK_SIZE 1024

struct List{
    int val;
    struct List* next;
};

void process(struct List* list, int cnt){
    //printf("%d\n", cnt);
    if(list->next != NULL)
	process(list->next, ++cnt);
}

void sethandler(void (*handler)(int,siginfo_t *,void *)){
    static char stack[SIGSTKSZ];
    struct sigaction sa;
    stack_t ss = {
        .ss_size = SIGSTKSZ,
	.ss_sp = stack,
    };
    memset(&sa, 0, sizeof(sigaction));
    //sigemptyset(&sa.sa_mask);
    sa.sa_flags     = SA_NODEFER|SA_ONSTACK;
    sa.sa_sigaction = handler;
    sigaltstack(&ss, 0);
    sigaction(SIGSEGV, &sa, NULL);
}

sigjmp_buf point;
void handler(int signo, siginfo_t *info, void *extra){
    longjmp(point, 1);
}

void main(void){
    sethandler(handler);
    struct List* list = malloc(sizeof(struct List));
    list->val = 1;
    list->next = list;
    if (setjmp(point) == 0)
        process(list, 0);
    else
	printf("Continue after segmentation fault\n");
}
