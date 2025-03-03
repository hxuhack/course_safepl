#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char *p[10];
    for(int i=0; i<10; i++){
        p[i] = malloc (10 * (i+1));
        strcpy(p[i], "nowar!!!");
    }

    for(int i=0; i<10; i++) {
        free(p[i]);
    }
    return 0;
}

