# Chapter 2. Allocator Design
## Section 2.1 Memory Management Overview
How does a process access the RAM? If there the RAM is uniquely owned by the process, it can directly read/write the RAM cells with the phisical address. However, if the RAM is shared among several processes, we should divide the phisical address space into different realms. This can be achieved either via direct segmentation (or unikernel) or virtual memory addressing. Due to the limitations in flexibility and security, unikernel is mainly used in embeded systems and LibOSes, while virtual memory addressing becomes the most popular way for PCs and Servers.
![image](./figures/chapt2-mem-2.png)

In vitual memory addressing, each process can only use the virtual address to access the memory. The virtual memory space of each process are exactly the same, and MMU on the Chip translates the virtual address of different processes to the corresponding phisical address automatically. This enforces the seperation of phisical memory usage. 
![image](./figures/chapt2-mem-3.png)

The following figure demonstrates a simple mapping between the virtual memory space and phisical memory space in some X86_64 systems (48-bit version). The virtual memory spaces of X86_64 ranges from 0x0000000000000000 - 0xffffffffffffffff. While memory addresses above 0xffff800000000000 are reserved for kernel usage, and addresses below 0x00007fffffffffff are reserved for user space. Since current X86_64 solutions mainly implements the 48-bit version, memory addresses in the middle are unused.The kernel space memory layout is the same for current processes running on the same operating system. As discribed in the document of ["Complete virtual memory map with 4-level page tables"](https://www.kernel.org/doc/Documentation/x86/x86_64/mm.txt), the kernel text is mapped to phisical address 0, and there is another kernel address range directly mapped to the phisical memory space. We will not go into the detailed mappings here. You only need to know the kernel code can easily calculate the physical address of a virtual memmory address in the kernel space via some offsets. 
![image](./figures/chapt2-mem-5.png)

You can check the virtual memory layout of a process throgh the /proc/pid/maps file and obtain some phisical memory layout information using the dmesg command.
```
#: cat /proc/pid/maps
#: dmesg
```

To facialiate the dynamic allocation and deallocation of virtual memories, the operating system provides a buddy system for memory management. As shown below, kernel code can use kmalloc() or vmalloc() to allocate memories through the buddy system. User space code should use system calls like brk and mmap to request memries through the buddy system. Such system calls are wrapped as allocation functions. Next, we elaborate the key challenges and solutions for kernel space memory management and user space memory management.
![image](./figures/chapt2-mem-man.png)

## Section 2.2 Kernel Space Allocator
The main functionality requirement of a kernel space allocator is to 1) allocate memory trunks based on the required size and 2) reclaim the memory once freed. Such operations should be done efficiently because kernel code is used by all processes. This is not difficult if not considering the fragmentation issues. However, as memory trunks are allocated and deallocated randomly, it will leave unused spaces between used trunks. If our allocation method tends to cut large unused trunks to meet the request of a small trunk, such unused spaces would become smaller and less reusable, knowing as fragmentation. To deal with the fragmentation issue, the allocation should be able to colase neighbor trunks if they are both free. The colasing algorithm should be efficient.

The buddy system is a well-designed page-level allocator that can meet these needs, i.e., the smallest allocation size is one or several pages.

### Allocation and Deallocation
In the buddy system, free blocks are managed as lists, where each list maintains the blocks of the same size. For example, there could be n lists, where each list maintains the blocks of size 2^i; the largest block is 2^n-1 pages, and the smallest block is 2^0 pages. The allocator employs a best fit strategy to search the blocks of the corresponding size from the list. If the best fit list is empty, it continually search other list of larger blocks and seperate the block into smaller ones for allocation. If all such lists are empty or unavailable, the allocator will allocate new pages from unused memory spaces.
![image](./figures/chapt2-buddy-3.png)

### Handle Fragmentation
If the allocator find a larger block during allocation, it needs to divide the block into smaller ones. The segmentation process is demonstrated as the following figure. Supossing the requested memory size is k bytes, the block (size 2^m) should be seperated in to halves n times until k>2^m-n-1. The rest blocks will be added back to the corresponding lists.
![image](./figures/chapt2-buddy-1.png)

Once the block is freed, the allocator immediately checks if its buddy is free and merge them if possible. The merging process is done recursively until the buddy is inuse. To faciliate the caculation of buddy addresses and the merging process, each block contains a header field as shown below. The TAG flag (1 bit) indicates whether the current block is allocated or free. The TYPE fields (2 bits) contains two bits to indicate whether the current block is a left or right buddy and whether its parrent is a left or righ buddy. The INDEX field (log n bits) contains the size information of the block. 
![image](./figures/chapt2-buddy-2.png)

### Demonstration
TODO: add an example to demonstrate the allocation and colasing process.

## Section 2.3 User Space Allocator
Similar to the kernel space allocator, a user space allocator should also be efficient and handle fragmentation issues.

To achieve efficient allocation, the user space allocator also maintains free lists for memory reuse. All freed memory trunks are added to the corresponding lists. This can avoid frequent context switch between the user space and kernel space which would slowdown the program. Different from the buddy system, user space memory blocks are generally small-sized. For example, in the Doug Lea's Allocator (a.k.a. [dlmalloc](https://github.com/ennorehling/dlmalloc/blob/master/malloc.c)), there are severl small regular bins with memory trunk size 16 bytes, 24 bytes, 32 bytes,...,512 bytes. Each regular bin is a double-linked list (to faciliate colasing). If a trunk is free, it contains a forward pointer and a backward pointer that link it to the bin. If it is not free, the pointer fields are used for data storage.
![image](./figures/chapt2-dlmalloc-1.png)

To handle fragmentation issues and achieve false colasing, each trunk contains a header field that allow the allocator to directly compute the address of contiguous trunks. As shown below, the header field contains 1) prev_size: size of the previous trunk to calculate the address of the previous trunk, 2) size: the size of the current trunk to calculate the address of the next trunk. Besides, it uses the last bit of the size to indicate whether the previous trunks is free or inuse. Note that because the minimal trunk size > 8 bytes, if representing the size information with a general integer, the last 3 bits is redundent. 
![image](./figures/chapt2-dlmalloc-2.png)

### Demonstration
TODO: add an example to demonstrate the colasing process.

## Practice
Implement a simple user space allocator based on the [code templete](./code/chapt2-allocator-template.c).
Hint:
- The template contains one free list.
- The trunk structure is defined as: 
```
struct chunk{
    unsigned long prev_size;
    unsigned long size; //use the last bit for prev_inuse
    struct chunk* fd;
    struct chunk* bk;
};
```
- We have initialized the list with a trunk of 1024 bytes memory.
```
void *p0 = sbrk(0);
brk(p0 + MEM_SIZE);
ty_chunk_ptr p = (ty_chunk_ptr) p0;
p->size = (unsigned long) MEM_SIZE | PREV_INUSE;
head = p;
p->bk = NULL;
p->fd = NULL;
```
- Your task is to implement the malloc_new() and free() function. The malloc_new() function find a trunk in the list for allocation. If trunk size > required sizem it splits it into two chunks. The free_new() adds the trunk back to the list and do consolidations if possible. 
- To check the correctness of your implementation, you can perform a few malloc and free operations and print the trunks.
```
void *x1 = malloc_new(8); 
void *x2 = malloc_new(16);
void *x3 = malloc_new(32);
void *x4 = malloc_new(48);
void *x5 = malloc_new(64);

view_chunk(p);

free_new(x1);
free_new(x2);
free_new(x3);
free_new(x4);
free_new(x5);

view_chunk(p); 
```



