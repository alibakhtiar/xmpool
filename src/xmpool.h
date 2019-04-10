/**
 * Nginx
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 *
 * The Nginx License
*/

/**
 * X-Memory-Pool
 *
 * Copyright (C) Ali Bakhtiar
 *
 * The Nginx License
 *
 * @modified : 21 January 2019
 * @created  : 03 July 2018
*/

#ifndef XMPOOL_H
#define XMPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>

typedef uint8_t    u_char;
typedef uintptr_t  xmpool_uint_t;

typedef struct xmpool_s  xmpool_t;
typedef struct xmpool_chain_s  xmpool_chain_t;

#ifndef XMPOOL_PAGESIZE
#define XMPOOL_PAGESIZE  (4 * 1024)
#endif

#ifndef XMPOOL_MAX_ALLOC_FROM_POOL
#define XMPOOL_MAX_ALLOC_FROM_POOL  (XMPOOL_PAGESIZE - 1)
#endif

#ifndef XMPOOL_DEFAULT_SIZE
#define XMPOOL_DEFAULT_SIZE    (16 * 1024)
#endif

#ifndef XMPOOL_ALIGNMENT_SIZE
#define XMPOOL_ALIGNMENT_SIZE   16
#endif

#ifndef XMPOOL_MIN_SIZE
#define XMPOOL_MIN_SIZE                                         \
    xmpool_align((sizeof(xmpool_t) + 2 * sizeof(xmpool_large_t)),  \
              XMPOOL_ALIGNMENT)
#endif

#define XMPOOL_ALIGNMENT  sizeof(unsigned long)

#define xmpool_memzero(buf, n)    (void) memset(buf, 0, n)

#define xmpool_align(d, a) (((d) + (a - 1)) & ~(a - 1))

#define xmpool_align_ptr(p, a)                                                  \
	(u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#define xmpool_malloc(size)  (void *)malloc(size)

#define xmpool_memalign(alignment, size)  xmpool_malloc(size)

typedef void (*xmpool_cleanup_pt)(void *data);

typedef struct xmpool_cleanup_s  xmpool_cleanup_t;

struct xmpool_cleanup_s {
	xmpool_cleanup_pt  handler;
	void               *data;
	xmpool_cleanup_t   *next;
};

typedef struct xmpool_large_s  xmpool_large_t;

struct xmpool_large_s {
	xmpool_large_t  *next;
	void            *alloc;
};

typedef struct {
	u_char        *last;
	u_char        *end;
	xmpool_t      *next;
	xmpool_uint_t failed;
} xmpool_data_t;


struct xmpool_s {
	xmpool_data_t     d;
	size_t            max;
	size_t            size;
	xmpool_t          *current;
	xmpool_chain_t    *chain;
	xmpool_large_t    *large;
	xmpool_cleanup_t  *cleanup;
};


xmpool_t *xmpool_new(size_t size);

void  xmpool_delete(xmpool_t *pool);

void  *xmpool_calloc(size_t size);

void  *xmpool_palloc(xmpool_t *pool, size_t size);

int   xmpool_free(xmpool_t *pool, void *p);

xmpool_cleanup_t *xmpool_cleanup_add(xmpool_t *p, size_t size);


#ifdef __cplusplus
}
#endif

#endif // XMPOOL_H