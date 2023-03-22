# Chapter 4. Memory Exhaustion and Handling
In this lecture, we study the issues of memory exhaustion, including stack overflow and heap overflow. Such issues would lead to unexpected thread or process termination. 
Even worse, some program exit points may lead to unreleased resources or consistency issues. 
For example, one thread could be killed before releasing its acquired lock or allocated memory. 
Althoug such issues are less harmful than buffer overflow or dangling pointers, we should not neglect such vulnerabilities for software systems with high reliability requirements.

## Section 3.1 Stack Overflow
The following code provides a function for reading lists. Can you construct a list to overflow the stack? 
```
struct List{
    int val;
    struct List* next;
};
//vulnerable function with stack overflow risks. 
void process(struct List* l, int cnt){
    printf("%d\n", cnt);
    if(l->next != NULL)
        process(l->next, ++cnt);
}
```
We can attack the function with a simple looped list.
```
void main(void){
    struct List* list = malloc(sizeof(struct List));
    list->val = 1;
    list->next = list;
    process(list, 0);
}

```
The stack size for each thread is generally limited. In this way, the address space of each thread can be well seperated. For example, the default stack size is 8MB in Linux. However, you can adjust the stack with the ulimit command.  

```
#: ulimit -s
stack size              (kbytes, -s) 8192
```

The stack address of each thread is determined by the stack size limit. For example, 
```
#: ulimit -s 65536
#: ulimit -a
stack size              (kbytes, -s) 65536
```

Besides, if developers think their program requies a large stack, they can adjust the stack size in their code, e.g., with the setrlimit() API in Linux.
```
struct rlimit r;
int result;
result = getrlimit(RLIMIT_STACK, &r);
fprintf(stderr, "stack result = %d\n", r.rlim_cur);
r.rlim_cur = 64 * 1024L *1024L;
result = setrlimit(RLIMIT_STACK, &r);
result = getrlimit(RLIMIT_STACK, &r);
fprintf(stderr, "stack result = %d\n", r.rlim_cur);
```

## Section 3.2 Heap Exhaustion
Heap allocation is not an atomic operation, such failures could occur in different stages. In generaly, the allocator should choose a address space for allocation at first. This step is unlikely to fail due to the large address space in X86_64. Meanwhile, the operating system may or may not allocate the phisical memory depending on its implementation. If there is not enough phisical memory, the malloc() function could return an error, and developers should check the return value before using the memory. However, the system may also employs a lazy mode, which is known as overcommit, i.e., do not allocate the phisical memory until it is accessed. In this way, the malloc() function is unlikely to fail. But if there is not enough phisical memory when accessed, the system would kill the process directly. Developers cannot handle such exceptions as easy as justifing the return value of malloc().

Linux has three options for the allocation behavior.
- 1: always overcommit, never check
- 2: always check, never overcommit
- 0: heuristic overcommit (this is the default)

We can set the allocation mode with the following command.
```
#: sudo sysctl -w vm.overcommit_memory=2
```

We can test the behaviors of the system with the following program. If we choose the never overcommit mode (option 2), malloc() returns 0. If we use the overcommit mode (option 1), malloc() returns a valid memory address. But the process would killed when we access the memory with memset().
```
#define LARGE_SIZE 1024L*1024L*1024L*256L
void main(void){
    char* p = malloc (LARGE_SIZE);
    if(p == 0) {
        printf("malloc failed\n");
    } else {
        memset (p, 1, LARGE_SIZE);
    }
}
```
```
#: sudo sysctl -w vm.overcommit_memory=2
#:~/4-memoxhaustion$ ./a.out
malloc failed
#: sudo sysctl -w vm.overcommit_memory=1
#:~/4-memoxhaustion$ ./a.out
Killed
```

In the overcommit mode, forgetting to check the return value of malloc is ok because the allocation is unlikely to fail. However, the process could be killed by the OS. To improve the usability of overcommit and mitigate the issue of killing a running process, Linux introduces a to-small-to-fail strategy, i.e., allocations of memory size less than a threshold should not fail. If there is not enough phisical memory, the operating system should kill other processes based on the badness of each process.

To demonstrate the effectiveness of to-small-to-fail, we can compare the results running the following program with/without overcommit. 
```
#define SMALL_SIZE 1024L
void exhaustheap() {
    for(long i=0; i < INT64_MAX; i++) {
        char* p = malloc (SMALL_SIZE);
        if(p == 0){
            printf("the %ldth malloc failed\n", i);
            break;
        } else {
            printf("access the %ldth memory chunk,...", i);
            memset (p, 0, sizeof (SMALL_SIZE));
            printf(", done\n", i);
        }
    }
}
```
According to the following results, the program can obtain more memory chunks than without the overcommit mode.  
```
#: sudo sysctl -w vm.overcommit_memory=2
#:~/4-memoxhaustion$ ./a.out
...
access the 2705176th memory chunk,..., done
the 2705177th malloc failed
#: sudo sysctl -w vm.overcommit_memory=1
#:~/4-memoxhaustion$ ./a.out
...
access the 9013022th memory chunk,..., done
Killed
```

## Section 3.3 Exception Handling
### OS Signal 
Can developers handle stack overflow or heap exhaustion bugs in their own code? It depends on the operating system. When stack overflow occurs, the operating system raises a SIGSEGV signal. It may allow the process to capture and handle the signal or killed the process directly. 

To simplify our discussion, we uses div 0 as an example. When the divisor is 0, the CPU will trigger an interrupt and executes the interrupt handling code specified in the interrupt vector of the operating system. The operating system then raises a signal SIGFPE (flating-point error). In the following code, we register the SIGFPE signal with the sigaction() API such that the code can jump to handle() when div 0.

```
void sethandler(void (*handler)(int,siginfo_t *,void *)){
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sigaction(SIGFPE, &sa, NULL);
}

void handler(int signo, siginfo_t *info, void *extra){
    printf("SIGFPE received!!!\n");
    exit(-1);
}

int main(void){
    sethandler(handler);
    int a = 0;
    int x = 100/a;
}
```

### setjmp/longjmp
How can we implement the handler function?


### Handle Stack Overflow
Handling the SIGSEV of stack overflow is much more difficult, because we do not have more stack spaces to execute the error handling code.


## Section 3.4 Stack Unwinding
Stack unwinding is an alternative approach of setjmp/longjmp. Instead of backup the register recovery info during runtime, it computes such information during compile time and save such info in the executables, i.e., the .eh_frame section of ELF file in DWARF format.

```
2690: endbr64
2694: push   %r15
2696: mov    %rsi,%rax
2699: push   %r14
269b: push   %r13
269d: push   %r12
269f: push   %rbp
26a0: push   %rbx
26a1: lea    0x4f94(%rip),%rbx        26a8: sub    $0x148,%rsp
26af: mov    %edi,0x2c(%rsp)
26b3: mov    (%rax),%rdi
…
27e7: sub    $0x8,%rsp
…
27fb: pushq  $0x0
…
2e96: pop    %rbx
2e97: pop    %rbp
2e98: pop    %r12
2e9a: pop    %r13
2e9c: pop    %r14
2e9e: pop    %r15
2ea0: retq
```
We can use the readelf tool to dump the .eh_frame section.
```
python3 pyelftools-master/scripts/readelf.py --debug-dump frames-interp /bin/cat
```

```
   LOC   CFA      rbx   rbp   r12   r13   r14   r15   ra
00002690 rsp+8    u     u     u     u     u     u     c-8
00002696 rsp+16   u     u     u     u     u     c-16  c-8
0000269b rsp+24   u     u     u     u     c-24  c-16  c-8
0000269d rsp+32   u     u     u     c-32  c-24  c-16  c-8
0000269f rsp+40   u     u     c-40  c-32  c-24  c-16  c-8
000026a0 rsp+48   u     c-48  c-40  c-32  c-24  c-16  c-8
000026a1 rsp+56   c-56  c-48  c-40  c-32  c-24  c-16  c-8
000026af rsp+384  c-56  c-48  c-40  c-32  c-24  c-16  c-8
000027eb rsp+392  c-56  c-48  c-40  c-32  c-24  c-16  c-8
000027fd rsp+400  c-56  c-48  c-40  c-32  c-24  c-16  c-8
00002825 rsp+384  c-56  c-48  c-40  c-32  c-24  c-16  c-8
00002e96 rsp+56   c-56  c-48  c-40  c-32  c-24  c-16  c-8
00002e97 rsp+48   c-56  c-48  c-40  c-32  c-24  c-16  c-8
00002e98 rsp+40   c-56  c-48  c-40  c-32  c-24  c-16  c-8
00002e9a rsp+32   c-56  c-48  c-40  c-32  c-24  c-16  c-8
00002e9c rsp+24   c-56  c-48  c-40  c-32  c-24  c-16  c-8
00002e9e rsp+16   c-56  c-48  c-40  c-32  c-24  c-16  c-8
00002ea0 rsp+8    c-56  c-48  c-40  c-32  c-24  c-16  c-8
```

