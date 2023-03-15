# Chapter 3. Memory Exhaustion and Handling
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

## Section 3.3 Exception Handling

## Section 3.4 Stack Unwinding
