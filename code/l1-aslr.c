#include <stdio.h>
void* getStack(){
   int ptr;
   printf("Stack pointer address: %p\n", &ptr);
   return __builtin_return_address(0);
};

int main(int argc, char** argv){
   printf("Return address: %p\n", getStack());
   int i;
   scanf("%d", &i);
   return 0;

}


