# Chapter 3. Heap Attack
Heap are managed by allocators. In the last chapter, we have already learnt the basic design of allocators. This chapter discusses how we can attack heap-related bugs based on specific allocators. In particular, we will focus on the user space allocator. 

## Section 3.1 Heap Analysis
Before going into the detailed attacking mechanism, we should know the allocator used in a target system. We can use the [GEF (GDB Enhanced Features) tool](https://hugsy.github.io/gef/) to faciliate our analysis. The tool has integrated heap analysis features which can interpret the allocated chunks and bins. We can use it to probe the runtime behavior of heap allocation and deallocation. 

```
int main(int argc, char** argv) {
    char *p[10];
    for(int i=0; i<10; i++){
        p[i] = malloc (10 * (i+1));
        strcpy(p[i], "nowar!!!");
    }

    for(int i=0; i<10; i++){
        free(p[i]);
    }
    return 0;
}
```
To probe the behavior of the allocator, we may a toy program with two for loops. The first loop allocates 10 memory chunks of different sizes 10-100, and then the second loop free these chunks iteratively. We can add breakpoints at the end of each loop body. 

After six iteration of the malloc loop. We can print the chunks via the command `heap chunks' There are eight chunks, the first one with addr 0x405010 is  used to store the metadata for heap management. The second one at 0x4052a0 is the memory allocated in the first loop. Note that we only requested a 10-byte chunk, but the allocated chunk size is 0x20 or 32 bytes. This implies the smallest chunk size is 0x20 in our system. Similary, the size of the next chunk is also 0x20, which can meet the requested memory size of 20 bytes. In the third iteration, the program requests a 30-byte chunk, the actually allacated chunk size is 0x30 bytes or 48 bytes. We can learn interval of chunk sizes managed by the allocator is 16 bytes. Note that there is a large chunk of 0x20c40 bytes at the end, and this is the remaining preallocated chunk. The allocator requrests for a large chunk in the first malloc function, all the rest mallocs would cut chunks from the large chunk until there is not enough space.

```
gef➤  heap chunks
Chunk(addr=0x405010, size=0x290, flags=PREV_INUSE)
    [0x0000000000405010     00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................]
Chunk(addr=0x4052a0, size=0x20, flags=PREV_INUSE)
    [0x00000000004052a0     6e 6f 77 61 72 21 21 21 00 00 00 00 00 00 00 00    nowar!!!........]
Chunk(addr=0x4052c0, size=0x20, flags=PREV_INUSE)
    [0x00000000004052c0     6e 6f 77 61 72 21 21 21 00 00 00 00 00 00 00 00    nowar!!!........]
Chunk(addr=0x4052e0, size=0x30, flags=PREV_INUSE)
    [0x00000000004052e0     6e 6f 77 61 72 21 21 21 00 00 00 00 00 00 00 00    nowar!!!........]
Chunk(addr=0x405310, size=0x30, flags=PREV_INUSE)
    [0x0000000000405310     6e 6f 77 61 72 21 21 21 00 00 00 00 00 00 00 00    nowar!!!........]
Chunk(addr=0x405340, size=0x40, flags=PREV_INUSE)
    [0x0000000000405340     6e 6f 77 61 72 21 21 21 00 00 00 00 00 00 00 00    nowar!!!........]
Chunk(addr=0x405380, size=0x50, flags=PREV_INUSE)
    [0x0000000000405380     6e 6f 77 61 72 21 21 21 00 00 00 00 00 00 00 00    nowar!!!........]
Chunk(addr=0x4053d0, size=0x20c40, flags=PREV_INUSE)
    [0x00000000004053d0     00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00    ................]
Chunk(addr=0x4053d0, size=0x20c40, flags=PREV_INUSE)  ←  top chunk
```

We can continue execute the program and probe the free lists. After nine iterations of the free loop. We can print the free list via the command `heap bins'. As shown below, there are five types of bins, tcachebins, fastbins, unsorted bin, small bins, and large bins. All our freed chunks are managed by the tcachebins, which is thread local. Each list of the tcachebins has a length limit (e.g., 7), and exceeding free chunks will be add to the fastbins.
Tcachebins is a single-lined list. Because its managed chunk sizes are small (<=0x80), the allocator does not need to the consider the consolidation issues. Besides, all the previous inuse bit is true no matter whether its previous chunk is free or not.


```
gef➤  heap bins
────────────────────────────────── Tcachebins for thread 1 ──────────────────────────────────
Tcachebins[idx=0, size=0x20, count=2] ←  Chunk(addr=0x4052c0, size=0x20, flags=PREV_INUSE)  ←  Chunk(addr=0x4052a0, size=0x20, flags=PREV_INUSE)
Tcachebins[idx=1, size=0x30, count=2] ←  Chunk(addr=0x405310, size=0x30, flags=PREV_INUSE)  ←  Chunk(addr=0x4052e0, size=0x30, flags=PREV_INUSE)
Tcachebins[idx=2, size=0x40, count=1] ←  Chunk(addr=0x405340, size=0x40, flags=PREV_INUSE)
Tcachebins[idx=3, size=0x50, count=2] ←  Chunk(addr=0x4053d0, size=0x50, flags=PREV_INUSE)  ←  Chunk(addr=0x405380, size=0x50, flags=PREV_INUSE)
Tcachebins[idx=4, size=0x60, count=1] ←  Chunk(addr=0x405420, size=0x60, flags=PREV_INUSE)
Tcachebins[idx=5, size=0x70, count=1] ←  Chunk(addr=0x405480, size=0x70, flags=PREV_INUSE)
─────────────────────────── Fastbins for arena at 0x7ffff7faeb80 ───────────────────────────
Fastbins[idx=0, size=0x20] 0x00
Fastbins[idx=1, size=0x30] 0x00
Fastbins[idx=2, size=0x40] 0x00
Fastbins[idx=3, size=0x50] 0x00
Fastbins[idx=4, size=0x60] 0x00
Fastbins[idx=5, size=0x70] 0x00
Fastbins[idx=6, size=0x80] 0x00
───────────────────────── Unsorted Bin for arena at 0x7ffff7faeb80 ─────────────────────────
[+] Found 0 chunks in unsorted bin.
────────────────────────── Small Bins for arena at 0x7ffff7faeb80 ──────────────────────────
[+] Found 0 chunks in 0 small non-empty bins.
────────────────────────── Large Bins for arena at 0x7ffff7faeb80 ──────────────────────────
[+] Found 0 chunks in 0 large non-empty bins.
```

We can display the raw data of the memory space. Because the header size of each trunk is 8 bytes, the address 0x405290 corresponds to the chunk at 0x4052a0.

```
gef➤  x/50x 0x405290
0x405298:       0x0000000000000021      0x0000000000000000
0x4052a8:       0x0000000000405010      0x0000000000000000
0x4052b8:       0x0000000000000021      0x00000000004052a0
0x4052c8:       0x0000000000405010      0x0000000000000000
0x4052d8:       0x0000000000000031      0x0000000000000000
0x4052e8:       0x0000000000405010      0x0000000000000000
0x4052f8:       0x0000000000000000      0x0000000000000000
0x405308:       0x0000000000000031      0x00000000004052e0
0x405318:       0x0000000000405010      0x0000000000000000
0x405328:       0x0000000000000000      0x0000000000000000
0x405338:       0x0000000000000041      0x0000000000000000
0x405348:       0x0000000000405010      0x0000000000000000
0x405358:       0x0000000000000000      0x0000000000000000
0x405368:       0x0000000000000000      0x0000000000000000
0x405378:       0x0000000000000051      0x0000000000000000
0x405388:       0x0000000000405010      0x0000000000000000
0x405398:       0x0000000000000000      0x0000000000000000
0x4053a8:       0x0000000000000000      0x0000000000000000
0x4053b8:       0x0000000000000000      0x0000000000000000
0x4053c8:       0x0000000000000051      0x0000000000405380
0x4053d8:       0x0000000000405010      0x0000000000000000
0x4053e8:       0x0000000000000000      0x0000000000000000
0x4053f8:       0x0000000000000000      0x0000000000000000
0x405408:       0x0000000000000000      0x0000000000000000
0x405418:       0x0000000000000061      0x0000000000000000
```

## Section 3.2 Heap Attack
To simplify our discussion, let's suppose the purpose of the attack is to obtain a pointer to an arbitrary address of our interest so that we can modify the data of the memory address. For example, we are interested in the return address because we may launch return-oriented programming attacks. We are also interested in the Global Offset Table (GOT) which maintains the addressing information of dynamic linked functions or position-independent code. For instance, wen may change the table entry of the widely used strcpy() function to a piece of malicious code. The similar method also applies to the Virtual Method Table (vtable) of C++/Rust code. 

To simplify the attacking scenario, we assume there is one free list. Therefore, all malloc() and free() operations will be performed based on the same list. In practice, there could be several bins, and each bin could have several lists of different sized chunks, which increases the complexity of attacks.

### 3.2.1 Heap Overflow Attack
Suppose p1 is a pointer to a heap chunk, and we can write beyond the chunk end through p1, i.e., with heap overflow issues. We may leverage the bug to modify the forward pointer of a following chunk if it is free. As shown in the figure below, the original forward pointer points to the next free chunk. To attack the bug, we can change the forward pointer to an arbitrary address of our interest. To this end, we should compute the offset of the forward pointer based on the size the current chunk. If the following chunk is not free, we should continue search other following chunks. 
![image](./figures/chapt3-heapoverflow-1.png)

Now, if we call the malloc(), it will remove the first chunk of the bin and point the bin head to the address of our interest.  
![image](./figures/chapt3-heapoverflow-2.png)

Calling the malloc() again would gain a pointer to the arbitrary address.
![image](./figures/chapt3-heapoverflow-3.png)

### 3.2.2 Use-After-Free Attack
Attacking use after free is similar to and even easier than attacking heap overflow. After freeing a pointer p1, p1 stills points to the freed chunk.
![image](./figures/chapt3-uaf-1.png)

Therefore, we can directly modify the forward pointer of the free chunks via p1. The rest steps are the same as those of the heap overflow attack. Different from heap overflow attack, we do not need to calculate the offset of the forward pointer based on p1.
![image](./figures/chapt3-uaf-2.png)

### 3.2.3 Double Free Attack
Suppose a free chunk pointed by p1 is already in the free list. Freeing p1 one more time executes the following link code.
```
p1->next = head->next;
head->next = p1;
```
As a result, it would incur a self-linked chunk and breaks the list.
![image](./figures/chapt3-doublefree-1.png)

Now we call malloc(), and it shoud unlink the chunk based on the following code.
```
p2 = head->next;
head->next = p2->next;
```
In this way, we obtain a pointer p2 to an allocatedchunk, but the chunk is also on the list. We can leverage p2 to modify the forward pointer of the chunk. The rest steps are the same as those of the previous attacks. 
![image](./figures/chapt3-doublefree-2.png)

## Section 2.3 Protection Techniques
