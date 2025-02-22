#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define PREV_INUSE 0b1 
#define chunksize(p) (((p)->size) & ~(PREV_INUSE))
#define next_chunk(p) ((struct chunk*) (((char *) (p)) + chunksize (p)))
#define prev_inuse(p) ((p)->size & PREV_INUSE)
#define prev_chunk(p) ((struct chunk*) (((char *) (p)) - prev_size (p)))
#define set_inuse(p) ((struct chunk*) (((char *) (p)) + chunksize (p)))->size |= PREV_INUSE
#define clear_inuse(p) ((struct chunk*) (((char *) (p)) + chunksize (p)))->size &= ~(PREV_INUSE)

// set the size of p with s
#define set_head_size(p, s)   ((p)->size = (s))

// set the prev_size of p's next_chunk with s
#define set_next_prev_size(p, s)  (((struct chunk*) ((char *) (p) + (s)))->prev_size = (s))

// next chunk which s is the size of p
#define chunk_at_offset(p, s)  ((struct chunk*) (((char *) (p)) + (s)))

// set the PREV_INUSE bit of the next chunk
#define set_previnuse_at_offset(p, s) (((struct chunk*) (((char *) (p)) + (s)))->size & PREV_INUSE)
  
// clear the PREV_INUSE bit of chunk after p
#define clear_previnuse_at_offset(p, s) (((struct chunk*) (((char *) (p)) + (s)))->size &= ~(PREV_INUSE))

// This is for x86_64; you can change it to 8 if you use x86
static uint64_t HEADER_SIZE = 16;

static uint64_t MIN_TRUNK_SIZE = 32;

// ptr without overhead
#define chunk2mem(p)   ((void*)((char*)(p) + HEADER_SIZE))
// ptr include overhead
#define mem2chunk(mem) ((struct chunk*)((char*)(mem) - HEADER_SIZE))

#define MEM_SIZE 1024 //define the init trunk size 

//The structure of chunk is a double linked list
struct chunk {
    uint64_t prev_size;	// size of previous chunk
    uint64_t size;		// size in bytes including overhead
    struct chunk* fd;		
    struct chunk* bk;
};

struct chunk* head;

void *new_malloc(uint64_t n){	
    struct chunk* p = head;
    struct chunk* remainder;
    uint64_t remainder_size;     
    n = n + HEADER_SIZE; //plus the header size
    
    while(1) {
	if(p == NULL){
	    puts("not enough space");
	    break;
	}
	if((uint64_t) chunksize(p) < n){ //search the next trunk
	    p = p->fd;
	    continue;
	}
	else { // trunks found
	    if ((uint64_t) chunksize(p) <= (uint64_t)(n + MIN_TRUNK_SIZE)) {	// it cannot be further split into two chunks;
	    if(p == head){
		head = p->fd;
                head->bk = NULL;
	    } else {
        	p->fd->bk = p->bk;
        	p->bk->fd = p->fd;
	    }
	        set_next_prev_size(p, chunksize(p));
                set_previnuse_at_offset(p, chunksize(p));
	    }
	    if ((uint64_t) chunksize(p) >= (uint64_t)(n + MIN_TRUNK_SIZE)) {
	    	remainder_size = chunksize(p) - n;
		remainder = chunk_at_offset (p, n);
		if(p == head) {
		    head = remainder;
                    remainder->bk = NULL;
		} else {
		    p->bk->fd = remainder;
		    remainder->bk = p->bk;
		}		
		set_head_size (p, n | prev_inuse(p));
		set_head_size (remainder, remainder_size | PREV_INUSE);
		remainder->prev_size = (uint64_t)(n);
		printf("chunk size = %d, prev_size = %d\n", p->size-prev_inuse(p), p->prev_size);
		set_next_prev_size(remainder,remainder_size);
		printf("remainder->prev_size = %d, reminder_size = %d\n", remainder->prev_size, remainder->size-prev_inuse(remainder));
	    }
	    break;	
	}
    }
    void *pp = chunk2mem(p);
    return pp;
}

void new_free(void *mem) {
    struct chunk* p = mem2chunk(mem);
    uint64_t size = chunksize(p);
    printf("free: p = %x\n", p);
    printf("chunk size = %d, prev_size = %d\n", p->size-prev_inuse(p), p->prev_size);
    clear_previnuse_at_offset(p, size);

    if (!prev_inuse(p)) {
	    //TODO: merge the block with its previous one
    }
    else {
	    //TODO: add the block to the free list 
    }
}

void view_chunk(struct chunk* p){
    void* bound = (void *) p + MEM_SIZE;
    printf("===============trunk view begin=====================\n");    
    printf("chunk addr = %p, prev size = %d, size: %d, prev inuse: %d, fd = %p, bk = %p\n", p, p->prev_size, p->size - prev_inuse(p), prev_inuse(p), p->fd, p->bk);
    p = (void *) p + p->size-prev_inuse(p);
    while(p < (void *) bound){
        printf("chunk addr = %p, prev size = %d, size: %d, prev inuse: %d, fd = %p, bk = %p\n", p, p->prev_size, p->size - prev_inuse(p), prev_inuse(p), p->fd, p->bk);
	p = (void *) p + p->size - prev_inuse(p);
    }
    printf("===============trunk view end=====================\n");    
}

int main() {
    // ---init----
    void *p0 = sbrk(0);
    brk(p0 + MEM_SIZE);
    struct chunk* p = (struct chunk*) p0;
    p->size = (uint64_t) MEM_SIZE | PREV_INUSE;
    head = p;
    struct chunk* init = p;
    p->bk = NULL;
    p->fd = NULL;
    printf("initialization done\n");
    printf("head address = %x\n", head);
    // ---init finished----
    void *x1 = new_malloc(8); 
    void *x2 = new_malloc(16);
    void *x3 = new_malloc(32);
    void *x4 = new_malloc(48);
    void *x5 = new_malloc(64);    
    view_chunk(init);

    new_free(x1);
    new_free(x2);
    new_free(x3);
    new_free(x4);
    new_free(x5);
    view_chunk(init);
} 
