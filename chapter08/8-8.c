/*
 * Exercise 8-8.  Write a routine bfree(p,n) that will free an arbitrary block
 * p of n characters into the free list maintained by malloc and free. By using
 * bfree, a user can add a static or external array to the free list at any
 * time.
 * By Faisal Saadatmand
 */

#define NULL        0
#define NALLOC      1024                /* minimum #units to request */
#define MAXBYTES    1048576            /* 1 Megabytes */

typedef long Align;                    /* for alignment to long boundary */

union header {                         /* block header: */
	struct {
		union header *ptr;             /* next block if on free list */
		unsigned size;                 /* size of this block */
	} s;
	Align x;                           /* force alignment of blocks */
};

typedef union header Header;

static Header base;                    /* empty list to get started */
static Header *freep = NULL;           /* start of free list */

void *knr_malloc(unsigned);
static Header *morecore(unsigned);
void knr_free(void *);

/* knr_malloc: general-purpose storage allocator */
void *knr_malloc(unsigned nbytes)
{
	Header *p;                         /* pointer to current block */
	Header *prevp;                     /* pointer to previous block */
	unsigned nunits;

	if (nbytes > MAXBYTES)             /* error check */
		return NULL;

	/* round up to allocate in units of sizeof(Header) */
	nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
	if ((prevp = freep) == NULL) {     /* no free list yet */
		base.s.ptr = &base;
		freep = prevp = &base;         /* point all to base */
		base.s.size = 0;
	}

	for (p = prevp->s.ptr; ; p = p->s.ptr) {    /* search free linked-list */
		if (p->s.size >= nunits) {     /* big enough */
			if (p->s.size == nunits)   /* exactly */
				prevp->s.ptr = p->s.ptr;
			else {
				p->s.size -= nunits;
				p += p->s.size;        /* allocate at tail-end */
				p->s.size = nunits;
			}
			freep = prevp;
			return (void *) (p + 1);
		}
		if (p == freep)                /* wrapped around free list */
			if ((p = morecore(nunits)) == NULL)            /* request more memory */
				return NULL;           /* none left */
		prevp = p;                     /* save current pointer's address */
	}
}

/* my_calloc: general-purpose storage allocator. Initialize memory to zeros */
void *my_calloc(unsigned  n, unsigned size)
{
	unsigned char *p;                  /* char is exactly 1 byte */
	unsigned i;

	if ((p = (unsigned char *) knr_malloc(n * size)) != NULL)
		for (i = 0; i < n * size; i++)
			p[i] &= 0x0u;              /* clear each byte */
	return (void *) p; 
}

/* morecore: ask system for more memory */
static Header *morecore(unsigned nu)
{
	char *cp;                          /* pointer to chunk of memory */
	char *sbrk(int);
	Header *up;

	if (nu < NALLOC)
		nu = NALLOC;
	cp = sbrk(nu * sizeof(Header));
	if (cp == (char *) -1)             /* no space at all */
		return NULL;
	up = (Header *) cp;
	up->s.size = nu;
	knr_free((void *)(up + 1));
	return freep;
}

/* knr_free: put block ap in free list */
void knr_free(void *ap)
{
	Header *bp, *p;
	int valid;

	valid = 1;
	if (ap == NULL)                        /* error checking */
		valid = 0;
	else {
		bp = (Header *) ap - 1;            /* point to block header */
		if (bp->s.size <= 1)        /* must be at least 2 units: */
			valid = 0;              /* 1 for header, 1 for mem block */ 
	}

	if (valid) {
		for (p = freep ; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
			if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
				break;              /* free block at start or end of arena */

		if (bp + bp->s.size == p->s.ptr) { /* join to upper nbr */
			bp->s.size += p->s.ptr->s.size;
			bp->s.ptr = p->s.ptr->s.ptr;
		} else
			bp->s.ptr = p->s.ptr;
		if (p + p->s.size == bp) {          /* join to lower nbr */
			p->s.size += bp->s.size;
			p->s.ptr = bp->s.ptr;
		} else
			p->s.ptr = bp;
		freep = p;
	}
}

/* bfree: free an arbitrary block p of n characters into the free list
 * maintained by malloc and free. n must be a multiple of size Header and at
 * least Header + 1. Return 0 on success, -1 on failure. */
int bfree(void *p, unsigned n)
{
	Header *hp;                        /* ptr to block header */
	unsigned nunits;                   /* number of units required */

	if (n <= sizeof(Header)            /* block doesn't fit header */
			|| (n % sizeof(Header)) != 0) /* or is not a multiple of 16 */
		return -1;

	nunits = n / sizeof(Header);       /* no rounding up is necessary */
	hp = (Header *) p;                 /* insert header at p[0] */
	hp->s.size = nunits;

	if (freep == NULL) {               /* no list yet. Create a degenerate list */
		base.s.ptr = freep = &base;
		base.s.size = 0;
	}

	knr_free((void *) (hp + 1));       /* add block to free list */

	return 0;
}

#include <stdio.h>

/* change these values to test bfree */
#define SIZE    320
#define SIZE2   160 
#define LIM     2                      /* to walk through the free list */
#define NCHAR   94                     /* number of ascii char to print */

int main(void)
{
	char array[SIZE];                /* statically allocated array */
	char *s;
	Header *p;
	int i;

	/* free array's mem block */
	if (bfree(array, sizeof(array)) < 0) {
		fprintf(stderr, "bfree: block size must be more than %d Bytes and a multiple of %d\n"
				, (int) sizeof(Header), (int) sizeof(Header));
	} else {
		printf("*** Add array's memory block to free list ***\n");
		for (p = &base, i = 1; i <= LIM; p = p->s.ptr, i++)
			printf("block %i (address: %p size: %u ptr: %p)\n"
					, i, p, p->s.size, p->s.ptr);
		printf("\n");
	}

	/* allocate freed memory */
	if ((s = (char *) knr_malloc(SIZE2 * sizeof(char))) == NULL)
		fprintf(stderr, "can't allocate memory\n");
	else {
		printf("*** allocate freed memory with knr_malloc ***\n");
		for (p = &base, i = 1; i <= LIM; p = p->s.ptr, i++)
			printf("block %i (address: %p size: %u ptr: %p)\n"
					, i, p, p->s.size, p->s.ptr);
		printf("\n");
	}
	
	/* initialize s's allocated memory */
	if (s != NULL) {
		printf("Test allocated memory:\n"); 
		for (i = 0; i <= NCHAR; i++)
			s[i] = i + 33;
		s[i] = '\0';
		printf("%s\n", s);
	}

	knr_free(array);
	knr_free(s);

	return 0;
}
