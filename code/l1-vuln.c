#include <stdio.h>
#include <unistd.h>

int validation() {
    int flag = 0;
    char buf[64];
    //printf("buf address: %p\n", (void*)buf);
    read(STDIN_FILENO, buf, 160);
    if(buf[0]=='$'){
        write(STDOUT_FILENO, "Key verified!\n", 14);
        flag = 1;
    }else{
    	write(STDOUT_FILENO, "Wrong key!\n", 11);
    }
    return flag;
}
int main(int argc, char** argv){
    int flag = 0;
    while(flag == 0){
    	write(STDOUT_FILENO, "Input your key:", 15);
        flag = validation();
    }
    printf("Start...\n");
}

