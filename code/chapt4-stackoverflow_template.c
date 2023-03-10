#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/resource.h>
#include <setjmp.h>
#include <signal.h>

struct List{
    int val;
    struct List* next;
};

void process(struct List* list, int cnt){
    printf("%d\n", cnt);
    if(list->next != NULL)
	process(list->next, ++cnt);
}

void sethandler(void (*handler)(int,siginfo_t *,void *)){
    //TODO: set an extra exeption handling stack with sigalstack();
    static char stack[SIGSTKSZ];
    stack_t ss = {
        .ss_size = SIGSTKSZ,
	.ss_sp = stack,
    };

    //TODO: register a signal handler with sigaction();
    struct sigaction sa;
    memset(&sa, 0, sizeof(sigaction));
    sa.sa_flags     = SA_NODEFER|SA_ONSTACK;
    sa.sa_sigaction = handler;
}

sigjmp_buf buf;
void handler(int signo, siginfo_t *info, void *extra){
    //TODO: implement the handler.
}

void main(void){
    sethandler(handler);
    struct List* list = malloc(sizeof(struct List));
    list->val = 1;
    list->next = list;
    if (setjmp(buf) == 0)
        process(list, 0);
    else
	printf("Continue after segmentation fault\n");
}
