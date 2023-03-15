# Chapter 3. Memory Exhaustion and Handling
In this lecture, we study the issues of memory exhaustion, including stack overflow and heap overflow. Such issues would lead to unexpected thread or process termination. 
Even worse, some program exit points may lead to unreleased resources or consistency issues. 
For example, one thread could be killed before releasing its acquired lock or allocated memory. 
Althoug such issues are less harmful than buffer overflow or dangling pointers, we should not neglect such vulnerabilities for software systems with high reliability requirements.

## Section 3.1 Stack Overflow

## Section 3.2 Heap Exhaustion

## Section 3.3 Exception Handling

## Section 3.4 Stack Unwinding
