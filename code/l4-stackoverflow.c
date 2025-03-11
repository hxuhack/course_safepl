#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

struct List{
    int val;
    struct List* next;
};

void process(struct List* list, int cnt){
    printf("%d\n", cnt);
    if(list->next != NULL)
	process(list->next, ++cnt);
}

void changesize(){
    struct rlimit r;
    int result;
    result = getrlimit(RLIMIT_STACK, &r);
    fprintf(stderr, "stack result = %d\n", r.rlim_cur);
    r.rlim_cur = 64 * 1024L *1024L;
    result = setrlimit(RLIMIT_STACK, &r);
    result = getrlimit(RLIMIT_STACK, &r);
    fprintf(stderr, "stack result = %d\n", r.rlim_cur);
}

void main(void){
    struct List* list = malloc(sizeof(struct List));
    list->val = 1;
    list->next = list;
    changesize();
    process(list, 0);
}
