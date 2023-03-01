/*
A toy allocator.
*/ 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PREV_INUSE 0b1 

#define chunksize(p) (((p)->size) & ~(PREV_INUSE))

// p's next chunk PTR
#define next_chunk(p) ((ty_chunk_ptr) (((char *) (p)) + chunksize (p)))

// p's prev inuse state
#define prev_inuse(p) ((p)->size & PREV_INUSE)

// Ptr to previous physical chunk.  Only valid if !prev_inuse (P). 
#define prev_chunk(p) ((ty_chunk_ptr) (((char *) (p)) - prev_size (p)))

// set/clear chunk as being inuse without otherwise disturbing 
#define set_inuse(p)                                                              \
  ((ty_chunk_ptr) (((char *) (p)) + chunksize (p)))->size |= PREV_INUSE
  
#define clear_inuse(p)                                                              \
  ((ty_chunk_ptr) (((char *) (p)) + chunksize (p)))->size &= ~(PREV_INUSE)

// set the size of p with s
#define set_head_size(p, s)   ((p)->size = (s))

// set the prev_size of p's next_chunk with s
#define set_foot(p, s)  (((ty_chunk_ptr) ((char *) (p) + (s)))->prev_size = (s))

// next chunk which s is the size of p
#define chunk_at_offset(p, s)  ((ty_chunk_ptr) (((char *) (p)) + (s)))

// if the next chunk inuse
#define set_previnuse_at_offset(p, s)                                              \
  (((ty_chunk_ptr) (((char *) (p)) + (s)))->size & PREV_INUSE)
  
// change a PREV_INUSE bit of chunk after p
#define clear_previnuse_at_offset(p, s)                                              \
  (((ty_chunk_ptr) (((char *) (p)) + (s)))->size &= ~(PREV_INUSE))

// for x86_64, the header size should be 16 for x86
static unsigned long HEADER_SIZE = 24;

// ptr without overhead
#define chunk2mem(p)   ((void*)((char*)(p) + HEADER_SIZE))
// ptr include overhead
#define mem2chunk(mem) ((ty_chunk_ptr)((char*)(mem) - HEADER_SIZE))

#define MEM_SIZE 1024 //define the init trunk size 

//The structure of chunk is a double linked list
struct chunk{
	unsigned long prev_size;	// size of previous chunk
	unsigned long size;		// size in bytes including overhead
	struct chunk* fd;		
	struct chunk* bk;
};
///ATTENTION///

typedef struct chunk* ty_chunk_ptr;
ty_chunk_ptr head;


void *malloc_new(unsigned long n){	
	ty_chunk_ptr p = head;
	ty_chunk_ptr remainder;              
	unsigned long remainder_size;     
	
	n = n + HEADER_SIZE; //plus the header size
	
	while(1){
		if(p == NULL){
			puts("not enough space");
			break;
		}
		if((unsigned long)chunksize(p) < n){ //search the next trunk
			p = p->fd;
			continue;
		}
		else if ((unsigned long)chunksize(p)<=(unsigned long)(n + HEADER_SIZE + 8)){			
			if(p==head){
				head = p->fd;
                                head->bk =NULL;
			}else {
        			p->fd->bk = p->bk;
        			p->bk->fd = p->fd;
			}
			set_foot(p,chunksize(p));
                        set_previnuse_at_offset(p,chunksize(p));
		}
		else if ((unsigned long)chunksize(p)>=(unsigned long)(n + HEADER_SIZE + 8)){			
			//TODO: cut the chunk into two parts and allocate the first part.
		}
		void *pp = chunk2mem(p);
		return pp;
	}
}

void free_new(void *mem) {
    ty_chunk_ptr p = mem2chunk(mem);
    unsigned long size = chunksize(p);
    clear_previnuse_at_offset(p, size);

    if (!prev_inuse(p)) {
	    //TODO: merge the block with its previous one
    }
    else {
	    //TODO: add the block to the free list 
    }
}

void view_chunk(ty_chunk_ptr p){
    void* bound = (void *) p + MEM_SIZE;
    printf("===============trunk view begin=====================\n");    
    printf("chunk address = %p, prev_size = %d, chunk size: %d, fd = %p, bk = %p\n", p, p->prev_size, p->size, p->fd, p->bk);
    p = (void *) p + p->size-prev_inuse(p);
    while(p < (void *) bound){
        printf("chunk address = %p, prev_size = %d, chunk size: %d, fd = %p, bk = %p\n", p, p->prev_size, p->size, p->fd, p->bk);
	p = (void *) p + p->size-prev_inuse(p);
    }
    printf("===============trunk view end=====================\n");    
}

void view_list(ty_chunk_ptr p){
    printf("===============list view begin=====================\n");    
    printf("chunk address = %p, prev_size = %d, chunk size: %d, fd = %p, bk = %p\n", p, p->prev_size, p->size, p->fd, p->bk);
    while(p->fd!=NULL){
	p = p->fd;
        printf("chunk address = %p, prev_size = %d, chunk size: %d, fd = %p, bk = %p\n", p, p->prev_size, p->size, p->fd, p->bk);
    }
    printf("===============list view end=====================\n");    
}

int main() {
    // ---init----
    void *p0 = sbrk(0);
    brk(p0 + MEM_SIZE);
    ty_chunk_ptr p = (ty_chunk_ptr) p0;
    p->size = (unsigned long) MEM_SIZE | PREV_INUSE;
    head = p;
    p->bk = NULL;
    p->fd = NULL;
    printf("initialization done\n");
    printf("head address = %x\n", p);
    // ---init finished----
    void *x1 = malloc_new(8); 
    void *x2 = malloc_new(16);
    void *x3 = malloc_new(32);
    void *x4 = malloc_new(48);
    void *x5 = malloc_new(64);    

    view_chunk(head);
    view_list(p);

    free_new(x1);
    free_new(x2);
    free_new(x3);
    free_new(x4);
    free_new(x5);

    view_chunk(p);
    view_list(p);
} 
