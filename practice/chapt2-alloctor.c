/*

You do not have to use syscall yourself.
The heap space has been applied and chunk_list is been initialized.

The other details of all the functions is in the ppt. Please read carefully.

*/ 

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// actually we don' need A&M
#define PREV_INUSE 0x1 //P
#define IS_MMAPPED 0x2 //M
#define NON_MAIN_ARENA 0x4 //A


// all these below is tools to read code easier
// p here is a mchunkptr which is malloc_chunk*

// bits used
#define SIZE_BITS (PREV_INUSE | IS_MMAPPED | NON_MAIN_ARENA)

// p's chunksize without bit P 
#define chunksize(p) (((p)->mchunk_size) & ~(SIZE_BITS))

// p's next chunk PTR
#define next_chunk(p) ((mchunkptr) (((char *) (p)) + chunksize (p)))

// p's prev inuse state
#define prev_inuse(p) ((p)->mchunk_size & PREV_INUSE)

// Size of the chunk below P. Only valid if !prev_inuse (P). 
#define prev_size(p) ((p)->mchunk_prev_size)

// Ptr to previous physical malloc_chunk.  Only valid if !prev_inuse (P). 
#define prev_chunk(p) ((mchunkptr) (((char *) (p)) - prev_size (p)))

// set/clear chunk as being inuse without otherwise disturbing 
#define set_inuse(p)                                                              \
  ((mchunkptr) (((char *) (p)) + chunksize (p)))->mchunk_size |= PREV_INUSE
  
#define clear_inuse(p)                                                              \
  ((mchunkptr) (((char *) (p)) + chunksize (p)))->mchunk_size &= ~(PREV_INUSE)

// reset the mchunk_size of p with s
#define set_head(p, s)       ((p)->mchunk_size = (s))

// reset the prev_mchunk_size of p's prev_chunk with s
#define set_foot(p, s)       (((mchunkptr) ((char *) (p) + (s)))->mchunk_prev_size = (s))

// next chunk which s is the size of p
#define chunk_at_offset(p, s)  ((mchunkptr) (((char *) (p)) + (s)))

// if the next chunk inuse
#define inuse_bit_at_offset(p, s)                                              \
  (((mchunkptr) (((char *) (p)) + (s)))->mchunk_size & PREV_INUSE)
  
// change a PREV_INUSE bit of chunk after p
#define clear_inuse_bit_at_offset(p, s)                                              \
  (((mchunkptr) (((char *) (p)) + (s)))->mchunk_size &= ~(PREV_INUSE))

// 64bit operating system
// SIZE_SZ = 8
//  
static unsigned long SIZE_SZ = 8;

// the overhead of a chunk is mchunk_prev_size and mchunk_size

// ptr without overhead
#define chunk2mem(p)   ((void*)((char*)(p) + 2*SIZE_SZ))
// ptr include overhead
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))



///ATTENTION///
struct malloc_chunk{
	
	unsigned long mchunk_prev_size;	// size of previous chunk
	
	unsigned long mchunk_size;		// size in bytes including overhead
	
	struct malloc_chunk* fd;// double links
	struct malloc_chunk* bk;
};
///ATTENTION///



typedef struct malloc_chunk* mchunkptr; // chunk pointer 

typedef struct malloc_chunk *mfastbinptr; //bin pointer


// malloc_state is used for a Controller which will be instantiated at the beginning of the program.
struct malloc_state{
	
	// chunk_list will have a tail which is empty at the beginning.
	mchunkptr chunk_list;
	// fastbin which is NULL at the beginning.
	mfastbinptr fastbin;
	
};

typedef struct malloc_state* mstate;


///ATTENTION///
// unlink_chunk could cut p out of the chunk_list or fastbin
void unlink_chunk(mstate av,mchunkptr p);
///ATTENTION///


///ATTENTION///
// merge function could make the chunk adjacent to p merge in to one chunk
// and chunk p won't automatically get into the chunklist
void merge(mstate av, mchunkptr p);
///ATTENTION///


// parameter n represents how much "Bytes" you want to get without overhead
// overhead is fixed 16B
// returns a pointer to the address you allocated
// default n = N*2*SIZE_EZ
// void *malloc_new() can cut a block from chunk_list or get one chunk out from fastbin.
// it will return a void*
void *malloc_new(mstate av,unsigned long n)
{
	
	mchunkptr p;
	// victim is the chunk be chosen to cut
	mchunkptr victim;
    // remainder from a split
	mchunkptr remainder;              
	unsigned long remainder_size;     
	
	n=n+2*SIZE_SZ;
	// request is small
	if(n <= 32 && av->fastbin != NULL) {
		p = av->fastbin;
		av->fastbin = p->fd;
		void *pp = chunk2mem(p);
		return pp;
	}// 
	else {
		victim = av->chunk_list;
		while(1){
			if(victim == NULL){
				puts("not enough space");
				break;
			}// place is not enough
			if((unsigned long)chunksize(victim)<(unsigned long)(n + SIZE_SZ*2+32)){
				victim = victim->fd;
				continue;
			}// enough
			else if ((unsigned long)chunksize(victim)>=(unsigned long)(n + SIZE_SZ*2+32)){
				
				remainder_size = chunksize(victim) - n;
				remainder = chunk_at_offset (victim, n);
				
				if(victim==av->chunk_list){
					av->chunk_list=remainder;
                    remainder->bk =NULL;
				}else {
					victim->bk->fd=remainder;
					remainder->bk=victim->bk;
				}
				
				set_head (victim, n | PREV_INUSE);
				set_head (remainder, remainder_size | PREV_INUSE);
				remainder->mchunk_prev_size = (unsigned long)(n);
                set_foot(remainder, remainder_size);

				void *pp = chunk2mem(victim);
				// don't have to initialize the memory
				return pp;
			}
		}
	}
	
	
}
// void *mem is the memomry you want free 
// if the chunk is small , it will be inserted back to fastbin
// if the chunk is big, it will be merge and back to chunklist
void free_new(mstate av, void *mem)
{
    unsigned long size;

    mchunkptr p;
    p = mem2chunk(mem);

    size = chunksize(p);

    if(size <= 32){
        if(av->fastbin != NULL){ 
            p->fd = av->fastbin;
            av->fastbin = p;
        }else{
            av->fastbin = p;
        }
    }
    else {
        merge(av, p);
        p->fd = av->chunk_list;
        p->fd->bk = p;
        p->bk = NULL;
        av->chunk_list = p;
    }
}

void merge(mstate av, mchunkptr p){

    unsigned long prevsize;
    unsigned long size;
    unsigned long nextsize;
    int nextinuse;
    mchunkptr nextchunk;
    mchunkptr lastchunk;

    size = chunksize(p);
    nextchunk = chunk_at_offset(p, size);
    nextsize = chunksize(nextchunk);
    nextinuse = inuse_bit_at_offset(nextchunk, nextsize);
    // please fulfill the function from here
    // merge forward
    



    // merge backward and you don't have to judge if the chunk out of range 



    // update p size



}

void unlink_chunk(mstate av, mchunkptr p){
        
    if (chunksize (p) != prev_size (next_chunk (p)))
        puts("corrupted size vs prev_size");
    // if p is the head
    if (p->bk == NULL){
        av->chunk_list = p->fd;
        p->fd->bk = NULL;
    }
    else {
        p->fd->bk = p->bk;
        p->bk->fd = p->fd;
    }
}

void init(mstate av, mchunkptr tail)
{
	tail->mchunk_size = (unsigned long)0;
	av->chunk_list = tail;
    av->fastbin = NULL;

}


int main()
{
    // ---init----
    struct malloc_chunk last;
    struct malloc_state controller;
    mstate av = &controller;
    mchunkptr tail = &last;
    init(av, tail);

    // syscall
    // initialize a chunk in the av->chunk_list
    void *p=sbrk(0);
    brk(p+320);
    tail->mchunk_prev_size = 320;
    mchunkptr pp = (mchunkptr)p;
    pp->mchunk_size = (unsigned long) 320 | PREV_INUSE;
    pp->bk = av->chunk_list;

    pp->bk->fd = pp;
    av->chunk_list = pp;
    // ---init finished----
    
    // test 
    void *x0 = malloc_new(av,16);
    
    void *x1 = malloc_new(av,32);

    void *x2 = malloc_new(av,48);

    void *x3 = malloc_new(av,64);
    
    void *x4 = malloc_new(av,16);
    
    

    free_new(av,x1);

    free_new(av,x3);

    free_new(av,x2);

    free_new(av,x0);

    free_new(av,x4);

    // check answer
    printf("%ld\n",av->chunk_list->mchunk_size);
    printf("%ld\n",av->chunk_list->fd->mchunk_size);

    printf("%ld\n",av->fastbin->mchunk_size);
    printf("%ld\n",av->fastbin->fd->mchunk_size);

    // the right answer is 193 65 32 33
    
	return 0;
} 
