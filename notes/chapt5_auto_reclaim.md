# Chapter 5. Memory Leakage And Memory Reclaim
In the last lecture, we have studied the issues of memory exhaustion. 
If excluding the system memory limit, memory leakage is an essential bug type that may lead to heap exhaustion.
In this lecture, we will discuss the memory leakage problem and three types of countermeasures to mitigate the issue.

## 5.1 Problem: Memory Leakage
The following code demonstrates a minimal memory leakage sample. It first mallocs a memory space and saves the address in p. If we forget to free the address pointer, the code would suffer memory leakage issue. In particular, the memory cell of p on the stack would be invalidated if reassign p or the function returns.

```
#define SMALL_SIZE 1024L
char* p = malloc (SMALL_SIZE);
...//forget call free(p)
p = malloc (SMALL_SIZE); //reassign p
```

## 5.2 Compile-time Approach


## 5.3 Smart Pointers

## 5.4 Garbage Collection
